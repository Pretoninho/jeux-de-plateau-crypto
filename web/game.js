/* game.js — UI web du jeu, pilotant le moteur C compilé en WebAssembly.
 * Aucune règle ici : tout passe par l'API wasm_* (moteur pur). */
"use strict";

const RES_NAME  = ["Rekt", "BTC", "ETH", "Alt", "Stable", "SOL"];
const RES_COLOR = ["#8b98ad", "#f7931a", "#627eea", "#8a63d2", "#26a17b", "#14f195"];
const RES_COUNT = 6;
const P_COLOR = ["#ef4444", "#3b82f6", "#22c55e", "#eab308"];
const P_NAME  = ["J1", "J2", "J3", "J4"];
const BUILD_MSG = {
  0: "construit ✔", 1: "joueur invalide", 2: "hors bornes", 3: "emplacement occupé",
  4: "trop proche d'une autre Position", 5: "non reliée à votre réseau de Lignes",
  6: "pas de Position à vous ici", 7: "ressources insuffisantes",
};

let M = null;            // module Emscripten
const F = {};            // fonctions wasm_* wrappées
let rolled = false;
let setup = { active: false, queue: [], idx: 0 };   // phase de mise en place

// Géométrie statique (recalculée à chaque nouvelle partie).
let TX = [], TY = [], VX = [], VY = [], EDGES = [];
let bounds = { minX: 0, minY: 0, maxX: 1, maxY: 1 };
const S = 46, PAD = 30;  // échelle pixel, marge

function wrapAll() {
  const sig = {
    wasm_new_game: [null, ["number", "number"]],
    wasm_n_players: ["number", []], wasm_current: ["number", []],
    wasm_initial_positions: ["number", []],
    wasm_place_free: ["number", ["number", "number"]],
    wasm_can_place_free: ["number", ["number", "number"]],
    wasm_n_tiles: ["number", []], wasm_n_vertices: ["number", []], wasm_n_edges: ["number", []],
    wasm_tile_resource: ["number", ["number"]], wasm_tile_number: ["number", ["number"]],
    wasm_tile_px: ["number", ["number"]], wasm_tile_py: ["number", ["number"]],
    wasm_vertex_px: ["number", ["number"]], wasm_vertex_py: ["number", ["number"]],
    wasm_vertex_building: ["number", ["number"]], wasm_vertex_owner: ["number", ["number"]],
    wasm_edge_v0: ["number", ["number"]], wasm_edge_v1: ["number", ["number"]],
    wasm_edge_owner: ["number", ["number"]],
    wasm_player_resource: ["number", ["number", "number"]], wasm_score: ["number", ["number"]],
    wasm_roll: ["number", []], wasm_end_turn: [null, []],
    wasm_build_position: ["number", ["number"]], wasm_build_line: ["number", ["number"]],
    wasm_build_desk: ["number", ["number"]],
    wasm_can_build_position: ["number", ["number"]], wasm_can_build_line: ["number", ["number"]],
    wasm_can_build_desk: ["number", ["number"]],
  };
  for (const name in sig) F[name] = M.cwrap(name, sig[name][0], sig[name][1]);
}

// Joueur actif : celui qui pose en phase de mise en place, sinon le joueur courant.
function activePlayer() { return setup.active ? setup.queue[setup.idx] : F.wasm_current(); }

// Ordre en serpentin : 0,1,..,n-1, n-1,..,1,0, … sur `rounds` tours.
function buildSetupQueue(n, rounds) {
  const q = [];
  for (let r = 0; r < rounds; r++) {
    if (r % 2 === 0) for (let p = 0; p < n; p++) q.push(p);
    else for (let p = n - 1; p >= 0; p--) q.push(p);
  }
  return q;
}

function readGeometry() {
  TX = []; TY = []; VX = []; VY = []; EDGES = [];
  const nt = F.wasm_n_tiles(), nv = F.wasm_n_vertices(), ne = F.wasm_n_edges();
  for (let i = 0; i < nt; i++) { TX.push(F.wasm_tile_px(i)); TY.push(F.wasm_tile_py(i)); }
  let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;
  for (let v = 0; v < nv; v++) {
    const x = F.wasm_vertex_px(v), y = F.wasm_vertex_py(v);
    VX.push(x); VY.push(y);
    if (x < minX) minX = x; if (x > maxX) maxX = x;
    if (y < minY) minY = y; if (y > maxY) maxY = y;
  }
  for (let e = 0; e < ne; e++) EDGES.push([F.wasm_edge_v0(e), F.wasm_edge_v1(e)]);
  bounds = { minX, minY, maxX, maxY };
}

const sx = (wx) => (wx - bounds.minX) * S + PAD;
const sy = (wy) => (wy - bounds.minY) * S + PAD;

function hexPoints(cx, cy, r) {
  const p = [];
  for (let i = 0; i < 6; i++) {
    const a = Math.PI / 180 * (60 * i);           // flat-top : coin à 0°
    p.push((cx + r * Math.cos(a)).toFixed(1) + "," + (cy + r * Math.sin(a)).toFixed(1));
  }
  return p.join(" ");
}

// Une intersection vide est-elle posable par le joueur actif ?
function vertexBuildable(v) {
  return setup.active
    ? F.wasm_can_place_free(activePlayer(), v) === 0
    : F.wasm_can_build_position(v) === 0;
}

function renderBoard() {
  const svg = document.getElementById("board");
  const w = (bounds.maxX - bounds.minX) * S + 2 * PAD;
  const h = (bounds.maxY - bounds.minY) * S + 2 * PAD;
  svg.setAttribute("viewBox", `0 0 ${w.toFixed(0)} ${h.toFixed(0)}`);

  let s = "";
  for (let i = 0; i < TX.length; i++) {
    const cx = sx(TX[i]), cy = sy(TY[i]);
    const res = F.wasm_tile_resource(i), num = F.wasm_tile_number(i);
    s += `<polygon class="hex" points="${hexPoints(cx, cy, S * 0.98)}" fill="${RES_COLOR[res]}" fill-opacity="0.82"/>`;
    s += `<text x="${cx}" y="${cy - S * 0.34}" text-anchor="middle" font-size="${(S * 0.28).toFixed(0)}" fill="#0b0e14" font-weight="700">${RES_NAME[res]}</text>`;
    if (num > 0) {
      const hot = (num === 6 || num === 8);
      s += `<circle cx="${cx}" cy="${cy + S * 0.12}" r="${(S * 0.30).toFixed(0)}" fill="#f4f1ea"/>`;
      s += `<text x="${cx}" y="${cy + S * 0.24}" text-anchor="middle" font-size="${(S * 0.34).toFixed(0)}" font-weight="700" fill="${hot ? "#d23b3b" : "#1a1a1a"}">${num}</text>`;
    }
  }
  for (let e = 0; e < EDGES.length; e++) {
    const [a, b] = EDGES[e];
    const x1 = sx(VX[a]), y1 = sy(VY[a]), x2 = sx(VX[b]), y2 = sy(VY[b]);
    const owner = F.wasm_edge_owner(e);
    const built = owner >= 0;
    const col = built ? P_COLOR[owner] : undefined;
    s += `<line data-edge="${e}" class="edge${built ? " built" : ""}" x1="${x1}" y1="${y1}" x2="${x2}" y2="${y2}" `
       + `stroke-width="${(S * 0.13).toFixed(1)}"${col ? ` stroke="${col}"` : ""}/>`;
  }
  for (let v = 0; v < VX.length; v++) {
    const cx = sx(VX[v]), cy = sy(VY[v]);
    const bd = F.wasm_vertex_building(v), ow = F.wasm_vertex_owner(v);
    if (bd === 1) {
      s += `<circle data-vtx="${v}" class="vtx" cx="${cx}" cy="${cy}" r="${(S * 0.22).toFixed(1)}" fill="${P_COLOR[ow]}" stroke="#0b0e14" stroke-width="1.5"/>`;
    } else if (bd === 2) {
      const r = S * 0.20;
      s += `<rect data-vtx="${v}" class="vtx" x="${(cx - r).toFixed(1)}" y="${(cy - r).toFixed(1)}" width="${(2 * r).toFixed(1)}" height="${(2 * r).toFixed(1)}" rx="3" fill="${P_COLOR[ow]}" stroke="#0b0e14" stroke-width="1.5"/>`;
    } else {
      if (vertexBuildable(v)) {
        s += `<circle class="buildable" cx="${cx}" cy="${cy}" r="${(S * 0.20).toFixed(1)}"><animate attributeName="opacity" values="0.35;0.9;0.35" dur="1.4s" repeatCount="indefinite"/></circle>`;
      }
      s += `<circle cx="${cx}" cy="${cy}" r="${(S * 0.08).toFixed(1)}" fill="#3a4557"/>`;
      s += `<circle data-vtx="${v}" class="vtx vtx-hit" cx="${cx}" cy="${cy}" r="${(S * 0.22).toFixed(1)}"/>`;
    }
  }
  svg.innerHTML = s;
}

function renderPanels() {
  const cur = activePlayer(), n = F.wasm_n_players();
  let s = "";
  for (let p = 0; p < n; p++) {
    s += `<div class="pcard${p === cur ? " active" : ""}">`
       + `<div class="top"><span class="dot" style="background:${P_COLOR[p]}"></span>`
       + `<span class="name">${P_NAME[p]}</span><span class="score">${F.wasm_score(p)} pt</span></div>`
       + `<div class="res">`;
    for (let k = 1; k < RES_COUNT; k++) {
      s += `<span><b style="color:${RES_COLOR[k]}">${RES_NAME[k]}</b> ${F.wasm_player_resource(p, k)}</span>`;
    }
    s += `</div></div>`;
  }
  document.getElementById("panels").innerHTML = s;
}

function updateTurnBar() {
  const w = activePlayer();
  const label = setup.active ? "Mise en place —" : "Tour de";
  document.getElementById("who").innerHTML =
    `${label} <span class="dot" style="background:${P_COLOR[w]}"></span> ${P_NAME[w]}`;

  const roll = document.getElementById("roll");
  const end = document.getElementById("end");
  const dice = document.getElementById("dice");
  roll.disabled = setup.active || rolled;
  end.disabled = setup.active;

  if (setup.active) {
    dice.innerHTML = `<small>placement ${setup.idx + 1}/${setup.queue.length}</small>`;
  } else if (!rolled) {
    dice.innerHTML = "<small>lancez les dés</small>";
  }
}

function render() { renderBoard(); renderPanels(); updateTurnBar(); }

function setMsg(t) { document.getElementById("msg").textContent = t; }

function onVertexClick(v) {
  if (setup.active) {
    const who = setup.queue[setup.idx];
    const r = F.wasm_place_free(who, v);
    if (r === 0) {
      setup.idx++;
      if (setup.idx >= setup.queue.length) {
        setup.active = false;
        setMsg("Mise en place terminée. Au tour de " + P_NAME[F.wasm_current()] + " — lancez les dés.");
      } else {
        setMsg("Position posée. Au tour de " + P_NAME[setup.queue[setup.idx]] + " de placer.");
      }
    } else {
      setMsg("Placement : " + BUILD_MSG[r]);
    }
    render();
    return;
  }

  const bd = F.wasm_vertex_building(v), ow = F.wasm_vertex_owner(v), cur = F.wasm_current();
  let r;
  if (bd === 0) { r = F.wasm_build_position(v); setMsg("Position : " + BUILD_MSG[r]); }
  else if (bd === 1 && ow === cur) { r = F.wasm_build_desk(v); setMsg("Desk : " + BUILD_MSG[r]); }
  else { setMsg("Intersection occupée."); return; }
  render();
}

function onEdgeClick(e) {
  if (setup.active) { setMsg("Placez d'abord vos Positions de départ."); return; }
  const r = F.wasm_build_line(e);
  setMsg("Ligne : " + BUILD_MSG[r]);
  render();
}

function newGame() {
  const players = parseInt(document.getElementById("players").value, 10);
  const seed = (parseInt(document.getElementById("seed").value, 10) || 0) >>> 0;
  F.wasm_new_game(players, seed);
  const rounds = F.wasm_initial_positions();
  setup = { active: true, queue: buildSetupQueue(players, rounds), idx: 0 };
  rolled = false;
  readGeometry();
  setMsg(`Mise en place : chaque joueur place ${rounds} Positions (cliquez une intersection verte). ${P_NAME[setup.queue[0]]} commence.`);
  render();
}

function doRoll() {
  if (setup.active || rolled) return;
  const d = F.wasm_roll();
  rolled = true;
  document.getElementById("dice").innerHTML = `🎲 <b>${d}</b>` + (d === 7 ? " <small>Margin Call</small>" : "");
  setMsg(d === 7 ? "7 — Margin Call (aucun effet en Phase 1)." : "Production distribuée.");
  render();
}

function endTurn() {
  if (setup.active) return;
  F.wasm_end_turn();
  rolled = false;
  setMsg("Au tour de " + P_NAME[F.wasm_current()] + ".");
  render();
}

function boot() {
  wrapAll();
  document.getElementById("board").addEventListener("click", (ev) => {
    const t = ev.target;
    if (t.dataset && t.dataset.vtx !== undefined) onVertexClick(parseInt(t.dataset.vtx, 10));
    else if (t.dataset && t.dataset.edge !== undefined) onEdgeClick(parseInt(t.dataset.edge, 10));
  });
  document.getElementById("newgame").addEventListener("click", newGame);
  document.getElementById("roll").addEventListener("click", doRoll);
  document.getElementById("end").addEventListener("click", endTurn);
  newGame();
}

createEngine().then((mod) => { M = mod; boot(); });
