/**
 * CANDYFLOSS TYCOON - FLOPPY EDITION (WEB APP)
 * Modern HTML5 / HiDPI Canvas / Web Audio API Edition
 * Inspired by Candyfloss (BBC Micro, 1982) by IJK Software
 */

// Application Constants & Enums
const APP_W = 960;
const APP_H = 600;
const PI = Math.PI;

const SCREEN_TITLE = 0;
const SCREEN_SETUP = 1;
const SCREEN_PASS_TURN = 2;
const SCREEN_SPIN = 3;
const SCREEN_PLAN = 4;
const SCREEN_RESULTS = 5;
const SCREEN_FINAL = 6;
const SCREEN_HOF = 7;
const SCREEN_ANALYSIS = 8;

const LANG_EN = 0;
const LANG_KO = 1;

const SOUND_CLICK = 0;
const SOUND_COIN = 1;
const SOUND_PROFIT = 2;
const SOUND_LOSS = 3;
const SOUND_WIN = 4;

const WEATHER_SUNNY = 0;
const WEATHER_BREEZY = 1;
const WEATHER_CLOUDY = 2;
const WEATHER_RAIN = 3;
const WEATHER_HEAT = 4;
const WEATHER_STORM = 5;

// Locations Data
const locationsData = [
  { nameEN: "Blackpool", nameKO: "블랙풀", pierName: "BLACKPOOL PIER", country: "UK", crowdMult: 1.15, priceSens: 1.00 },
  { nameEN: "Skegness", nameKO: "스케그니스", pierName: "SKEGNESS PIER", country: "UK", crowdMult: 1.05, priceSens: 0.95 },
  { nameEN: "Clacton-on-Sea", nameKO: "클랙턴온시", pierName: "CLACTON PIER", country: "UK", crowdMult: 1.00, priceSens: 0.90 },
  { nameEN: "Brighton Pier", nameKO: "브라이튼", pierName: "BRIGHTON PIER", country: "UK", crowdMult: 1.10, priceSens: 1.25 },
  { nameEN: "Jeju Island", nameKO: "제주도", pierName: "JEJU PARADISE", country: "KO", crowdMult: 1.20, priceSens: 1.10 },
  { nameEN: "Haeundae Beach", nameKO: "부산 해운대", pierName: "HAEUNDAE BEACH", country: "KO", crowdMult: 1.30, priceSens: 1.05 },
  { nameEN: "Gangneung Beach", nameKO: "강릉 안목해변", pierName: "GANGNEUNG BEACH", country: "KO", crowdMult: 1.08, priceSens: 1.15 }
];

const stallColours = [
  "rgb(244, 63, 142)",  // Pink
  "rgb(37, 150, 235)",  // Blue
  "rgb(245, 174, 34)",  // Gold
  "rgb(22, 185, 138)"   // Mint
];

const stallDarkColours = [
  "rgb(190, 24, 93)",
  "rgb(29, 78, 216)",
  "rgb(180, 110, 9)",
  "rgb(13, 132, 98)"
];

const playerNamesEN = ["Player 1", "Player 2", "Player 3", "Player 4"];
const playerNamesKO = ["플레이어 1", "플레이어 2", "플레이어 3", "플레이어 4"];

const defaultBizNamesEN = [
  "Pink Cloud Floss", "Ocean Breeze Sweets", "Golden Sun Sweets",
  "Mint Pier Paradise", "Beachfront Candy Co.", "Royal Pier Treats"
];
const defaultBizNamesKO = [
  "분홍 구름 솜사탕", "바다 바람 솜사탕", "황금 햇살 솜사탕",
  "민트 피어 솜사탕", "해변길 솜사탕 상회", "로얄 피어 디저트"
];

const beachZoneNamesEN = ["North Promenade", "South Esplanade", "East Dunes Plaza", "West Coast Strip"];
const beachZoneNamesKO = ["북쪽 피어 산책로", "남쪽 해변 산책로", "동쪽 언덕 광장", "서쪽 해안 거리"];

const weatherNamesEN = ["Sunny", "Warm & Breezy", "Cloudy", "Light Rain", "Heatwave", "Storm"];
const weatherNamesKO = ["맑음", "화창하고 바람", "흐림", "가벼운 비", "폭염", "폭풍우"];
const weatherDemand = [1.18, 1.05, 0.86, 0.68, 1.48, 0.42];

const eventNamesEN = ["Normal Day", "School Trip", "Donkey Trouble", "Seagull Invasion", "Festival Parade", "Sugar Delay", "Perfect Reviews"];
const eventNamesKO = ["평범한 하루", "수학여행 단체 방문", "장난꾸러기 당나귀", "갈매기떼의 습격", "축제 퍼레이드", "설탕 배송 지연", "최고의 맛집 리뷰"];

const eventTextsEN = [
  "Nothing unusual happens. Your decisions do the talking.",
  "A coach full of pupils arrives at lunchtime.",
  "A mischievous donkey eats some prepared stock.",
  "Seagulls frighten customers away for part of the afternoon.",
  "A colourful parade brings extra visitors past your stall.",
  "A late sugar delivery raises production costs.",
  "A local reviewer praises your candyfloss."
];
const eventTextsKO = [
  "특별한 일 없이 평범한 하루입니다. 경영 전략에 집중하세요.",
  "점심시간에 학생들을 태운 버스가 도착했습니다.",
  "장난꾸러기 당나귀가 준비해둔 재고 일부를 먹어치웠습니다.",
  "갈매기떼가 나타나 오후 동안 손님들을 쫓아냈습니다.",
  "화려한 퍼레이드로 많은 관람객이 가게 앞을 지나갑니다.",
  "설탕 배송이 지연되어 원가 비용이 상승했습니다.",
  "지역 맛집 평가원이 당신의 솜사탕을 극찬했습니다."
];

// Sound Synthesizer (Web Audio API)
let audioCtx = null;

function initAudio() {
  if (!audioCtx) {
    audioCtx = new (window.AudioContext || window.webkitAudioContext)();
  }
  if (audioCtx && audioCtx.state === "suspended") {
    audioCtx.resume();
  }
}

function playBeep(freq, durationMs, delayMs = 0) {
  if (!g.soundOn || !audioCtx) return;
  setTimeout(() => {
    try {
      const osc = audioCtx.createOscillator();
      const gain = audioCtx.createGain();
      osc.type = "triangle";
      osc.frequency.value = freq;
      gain.gain.setValueAtTime(0.09, audioCtx.currentTime);
      gain.gain.exponentialRampToValueAtTime(0.001, audioCtx.currentTime + durationMs / 1000);
      osc.connect(gain);
      gain.connect(audioCtx.destination);
      osc.start();
      osc.stop(audioCtx.currentTime + durationMs / 1000);
    } catch (e) {}
  }, delayMs);
}

function playSound(type) {
  initAudio();
  if (!g.soundOn) return;
  if (type === SOUND_CLICK) {
    playBeep(880, 28);
  } else if (type === SOUND_COIN) {
    playBeep(987, 45);
    playBeep(1318, 95, 45);
  } else if (type === SOUND_PROFIT) {
    playBeep(523, 70, 0);
    playBeep(659, 70, 75);
    playBeep(784, 70, 150);
    playBeep(1046, 150, 225);
  } else if (type === SOUND_LOSS) {
    playBeep(440, 90, 0);
    playBeep(370, 90, 95);
    playBeep(310, 160, 190);
  } else if (type === SOUND_WIN) {
    playBeep(523, 80, 0);
    playBeep(659, 80, 85);
    playBeep(784, 80, 170);
    playBeep(1046, 120, 255);
    playBeep(880, 80, 380);
    playBeep(1046, 250, 465);
  }
}

// Game State Definition
class PlayerState {
  constructor(stallColour = 0, defaultName = "Candy Stall") {
    this.active = 1;
    this.businessName = defaultName;
    this.cashPence = 3500;
    this.debtPence = 0;
    this.reputation = 50;
    this.stock = 40;
    this.pricePence = 250;
    this.adverts = 3;
    this.bonusStock = 0;
    this.sold = 0;
    this.customers = 0;
    this.unsold = 0;
    this.revenuePence = 0;
    this.costPence = 0;
    this.profitPence = 0;
    this.totalSold = 0;
    this.totalStockMade = 0;
    this.bestProfit = -999999;
    this.stallColour = stallColour;
  }
}

const g = {
  screen: SCREEN_TITLE,
  lang: LANG_EN,
  loc: 0,
  numPlayers: 1,
  currentPlayer: 0,
  day: 1,
  totalDays: 10,
  difficulty: 1, // 0: Easy, 1: Normal, 2: Hard
  soundOn: true,
  unitCostPence: 45,
  suggestedPricePence: 250,
  crowd: 100,
  weather: WEATHER_SUNNY,
  eventId: 0,
  players: [
    new PlayerState(0, defaultBizNamesEN[0]),
    new PlayerState(1, defaultBizNamesEN[1]),
    new PlayerState(2, defaultBizNamesEN[2]),
    new PlayerState(3, defaultBizNamesEN[3])
  ],
  highScores: [],
  highScoreRecorded: false,
  editingNamePlayer: -1,

  // Spin mini-game state
  spinStartTick: 0,
  lastMouseAngle: 0,
  spinTotalAngle: 0,
  bonusEarned: 0,
  spinTrail: [],
  spinParticles: [],

  // Pointer state
  mouseDown: false,
  activeSlider: -1,
  hoverX: -1,
  hoverY: -1,
  tick: 0
};

// UI Element Coordinates
const titleButtons = [
  { r: { left: 350, top: 220, right: 610, bottom: 268 }, text: "PLAY GAME", id: 1 },
  { r: { left: 350, top: 280, right: 610, bottom: 328 }, text: "HOW TO PLAY", id: 2 },
  { r: { left: 350, top: 340, right: 610, bottom: 388 }, text: "HALL OF FAME", id: 3 },
  { r: { left: 350, top: 400, right: 610, bottom: 448 }, text: "SOUND: ON", id: 4 },
  { r: { left: 350, top: 460, right: 610, bottom: 508 }, text: "LANG: EN", id: 5 }
];

const titleInspirationBtn = {
  r: { left: 255, top: 534, right: 705, bottom: 572 },
  id: 6
};

const setupButtons = [
  { r: { left: 330, top: 522, right: 630, bottom: 574 }, text: "START GAME", id: 10 },
  { r: { left: 32, top: 24, right: 142, bottom: 62 }, text: "BACK", id: 11 }
];

const passButtons = [
  { r: { left: 310, top: 424, right: 650, bottom: 482 }, text: "START FLOSS SPINNING", id: 15 }
];

const planButtons = [
  { r: { left: 668, top: 514, right: 932, bottom: 572 }, text: "OPEN FOR BUSINESS", id: 20 },
  { r: { left: 668, top: 416, right: 794, bottom: 454 }, text: "BORROW £50", id: 21 },
  { r: { left: 806, top: 416, right: 932, bottom: 454 }, text: "REPAY £50", id: 22 }
];

const resultButtons = [
  { r: { left: 330, top: 504, right: 630, bottom: 558 }, text: "NEXT PLAYER", id: 30 }
];

const finalButtons = [
  { r: { left: 225, top: 504, right: 465, bottom: 558 }, text: "PLAY AGAIN", id: 40 },
  { r: { left: 495, top: 504, right: 735, bottom: 558 }, text: "PERFORMANCE REPORT", id: 41 }
];

const hofButtons = [
  { r: { left: 350, top: 506, right: 610, bottom: 560 }, text: "BACK TO TITLE", id: 50 }
];

const analysisButtons = [
  { r: { left: 350, top: 514, right: 610, bottom: 566 }, text: "BACK TO LEADERBOARD", id: 60 }
];

const sliders = [
  { r: { left: 675, top: 218, right: 925, bottom: 248 }, minv: 10, maxv: 250, value: 80, id: 1 },
  { r: { left: 675, top: 292, right: 925, bottom: 322 }, minv: 100, maxv: 500, value: 250, id: 2 },
  { r: { left: 675, top: 366, right: 925, bottom: 396 }, minv: 0, maxv: 20, value: 3, id: 3 }
];

// Helper Functions
function irand(a, b) {
  return a + Math.floor(Math.random() * (b - a + 1));
}

function clampi(v, lo, hi) {
  return v < lo ? lo : v > hi ? hi : v;
}

function ptIn(r, x, y) {
  return x >= r.left && x <= r.right && y >= r.top && y <= r.bottom;
}

function isHover(r) {
  return ptIn(r, g.hoverX, g.hoverY);
}

function moneyStr(pence) {
  const neg = pence < 0;
  if (neg) pence = -pence;
  const pounds = Math.floor(pence / 100);
  const p = (pence % 100).toString().padStart(2, '0');
  return `${neg ? '-' : ''}£${pounds}.${p}`;
}

// LocalStorage High Scores
function loadHighScores() {
  try {
    const saved = localStorage.getItem("candyfloss_highscores");
    if (saved) {
      g.highScores = JSON.parse(saved);
    } else {
      g.highScores = [
        { pName: "Master Dan", bName: "Golden Cloud", locName: "Blackpool Pier", score: 14500, cashPence: 9500, reputation: 90, totalSold: 350 },
        { pName: "Seaside Sam", bName: "Brighton Sweets", locName: "Brighton Pier", score: 11200, cashPence: 7200, reputation: 78, totalSold: 280 },
        { pName: "Jeju Queen", bName: "Paradise Floss", locName: "Jeju Island", score: 8900, cashPence: 5800, reputation: 65, totalSold: 220 },
        { pName: "Pier Trader", bName: "Ocean Floss", locName: "Skegness Pier", score: 6500, cashPence: 4200, reputation: 55, totalSold: 180 },
        { pName: "Novice Nick", bName: "Mint Floss", locName: "Clacton Pier", score: 4100, cashPence: 2900, reputation: 45, totalSold: 110 }
      ];
    }
  } catch (e) {
    g.highScores = [];
  }
}

function saveHighScores() {
  try {
    localStorage.setItem("candyfloss_highscores", JSON.stringify(g.highScores));
  } catch (e) {}
}

function checkAndAddHighScore(pName, bName, locName, score, cashPence, rep, totalSold) {
  let insertIdx = -1;
  for (let i = 0; i < 5; i++) {
    if (!g.highScores[i] || score > g.highScores[i].score) {
      insertIdx = i;
      break;
    }
  }
  if (insertIdx >= 0) {
    g.highScores.splice(insertIdx, 0, {
      pName, bName, locName, score, cashPence, reputation: rep, totalSold
    });
    g.highScores = g.highScores.slice(0, 5);
    saveHighScores();
  }
}

// Performance Analyzer
function analyzePlayerPerformance(pIdx) {
  const p = g.players[pIdx];
  const netWealth = p.cashPence - p.debtPence;
  const score = netWealth + p.reputation * 45 + p.totalSold * 5;
  const eff = p.totalStockMade > 0 ? Math.floor((p.totalSold / p.totalStockMade) * 100) : 0;

  let grade = "C", title = "", tip = "";
  if (score >= 12000 && netWealth >= 8000) {
    grade = "S";
    title = g.lang === LANG_EN ? "Master Tycoon" : "전설의 마스터 타이쿤";
  } else if (score >= 9000) {
    grade = "A";
    title = g.lang === LANG_EN ? "Seaside Executive" : "해변의 특급 경영자";
  } else if (score >= 6000) {
    grade = "B";
    title = g.lang === LANG_EN ? "Profitable Trader" : "수익형 상인";
  } else if (score >= 3000) {
    grade = "C";
    title = g.lang === LANG_EN ? "Struggling Vendor" : "노력하는 노점상";
  } else {
    grade = "D";
    title = g.lang === LANG_EN ? "Bankrupt Vendor" : "파산 위기 노점상";
  }

  if (p.debtPence > 3000) {
    tip = g.lang === LANG_EN ? "High debt interest ate your profits. Repay bank loans early!" : "대출 이자가 수익을 많이 깎아먹었습니다. 대출금을 조기에 상환하세요!";
  } else if (eff < 65) {
    tip = g.lang === LANG_EN ? "High unsold stock waste. Reduce production on rainy or storm days!" : "남은 재고 낭비가 컸습니다. 비나 폭풍우가 치는 날에는 제조량을 줄이세요!";
  } else if (p.totalSold < 150) {
    tip = g.lang === LANG_EN ? "Sales volume was low. Try buying more advertising signs!" : "판매량이 적었습니다. 홍보 표지판을 늘려 고객 유동인구를 유인하세요!";
  } else {
    tip = g.lang === LANG_EN ? "Excellent balance of pricing, advertising, and inventory control!" : "가격 설정, 홍보, 재고 관리의 훌륭한 경영 균형을 보여주었습니다!";
  }

  return { grade, title, tip, eff };
}

// Canvas Graphics System Context
let canvas, ctx;
let currentDpr = 1;

function syncCanvasResolution() {
  const dpr = Math.min(window.devicePixelRatio || 1, 2);
  const targetW = Math.round(APP_W * dpr);
  const targetH = Math.round(APP_H * dpr);
  if (canvas.width !== targetW || canvas.height !== targetH || currentDpr !== dpr) {
    canvas.width = targetW;
    canvas.height = targetH;
    currentDpr = dpr;
  }
  ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
}

function adjustColor(colorStr, percent) {
  let num = parseInt(colorStr.replace("#", ""), 16);
  if (isNaN(num)) {
    const match = colorStr.match(/\d+/g);
    if (!match) return colorStr;
    const r = clampi(parseInt(match[0]) + percent, 0, 255);
    const gg = clampi(parseInt(match[1]) + percent, 0, 255);
    const b = clampi(parseInt(match[2]) + percent, 0, 255);
    return `rgb(${r}, ${gg}, ${b})`;
  }
  const amt = Math.round(2.55 * percent);
  const R = clampi((num >> 16) + amt, 0, 255);
  const G = clampi(((num >> 8) & 0x00FF) + amt, 0, 255);
  const B = clampi((num & 0x0000FF) + amt, 0, 255);
  return `rgb(${R}, ${G}, ${B})`;
}

// Drawing Utilities
function fillRound(x, y, w, h, radius, fillStyle) {
  ctx.fillStyle = fillStyle;
  ctx.beginPath();
  ctx.roundRect(x, y, w, h, radius);
  ctx.fill();
}

function strokeRound(x, y, w, h, radius, strokeStyle, lineWidth = 1.5) {
  ctx.strokeStyle = strokeStyle;
  ctx.lineWidth = lineWidth;
  ctx.beginPath();
  ctx.roundRect(x, y, w, h, radius);
  ctx.stroke();
}

function fillGradRound(x, y, w, h, radius, topCol, botCol) {
  const grad = ctx.createLinearGradient(x, y, x, y + h);
  grad.addColorStop(0, topCol);
  grad.addColorStop(1, botCol);
  fillRound(x, y, w, h, radius, grad);
}

function line(x1, y1, x2, y2, strokeStyle, lineWidth = 1, cap = "round") {
  ctx.strokeStyle = strokeStyle;
  ctx.lineWidth = lineWidth;
  ctx.lineCap = cap;
  ctx.beginPath();
  ctx.moveTo(x1, y1);
  ctx.lineTo(x2, y2);
  ctx.stroke();
}

function ellipseFill(cx, cy, rx, ry, fillStyle) {
  ctx.fillStyle = fillStyle;
  ctx.beginPath();
  ctx.ellipse(cx, cy, rx, ry, 0, 0, Math.PI * 2);
  ctx.fill();
}

function polygonFill(pts, fillStyle) {
  ctx.fillStyle = fillStyle;
  ctx.beginPath();
  ctx.moveTo(pts[0].x, pts[0].y);
  for (let i = 1; i < pts.length; i++) {
    ctx.lineTo(pts[i].x, pts[i].y);
  }
  ctx.closePath();
  ctx.fill();
}

function centreText(x1, y1, x2, y2, str, color, font) {
  ctx.font = font;
  ctx.fillStyle = color;
  ctx.textAlign = "center";
  ctx.textBaseline = "middle";
  ctx.fillText(str, (x1 + x2) / 2, (y1 + y2) / 2);
}

function textLeft(x, y, str, color, font) {
  ctx.font = font;
  ctx.fillStyle = color;
  ctx.textAlign = "left";
  ctx.textBaseline = "top";
  ctx.fillText(str, x, y);
}

function drawCardPanel(x, y, w, h, r = 28) {
  ctx.save();
  ctx.shadowColor = "rgba(15, 23, 42, 0.35)";
  ctx.shadowBlur = 32;
  ctx.shadowOffsetY = 14;
  fillRound(x, y, w, h, r, "rgba(255, 255, 255, 0.97)");
  ctx.restore();
  strokeRound(x, y, w, h, r, "rgba(255, 255, 255, 0.65)", 1.5);
}

function drawModernBackdrop(topCol, midCol, botCol) {
  const grad = ctx.createLinearGradient(0, 0, 0, APP_H);
  grad.addColorStop(0, topCol);
  grad.addColorStop(0.55, midCol);
  grad.addColorStop(1, botCol);
  ctx.fillStyle = grad;
  ctx.fillRect(0, 0, APP_W, APP_H);

  // Subtle ambient coastal bokeh lights
  ctx.save();
  for (let i = 0; i < 6; i++) {
    const bx = 90 + i * 165 + Math.sin(g.tick * 0.008 + i) * 18;
    const by = 85 + (i % 2) * 380 + Math.cos(g.tick * 0.006 + i) * 12;
    const br = 90 + (i % 3) * 35;
    const bg = ctx.createRadialGradient(bx, by, 4, bx, by, br);
    bg.addColorStop(0, "rgba(255, 255, 255, 0.12)");
    bg.addColorStop(1, "rgba(255, 255, 255, 0)");
    ctx.fillStyle = bg;
    ctx.beginPath();
    ctx.arc(bx, by, br, 0, PI * 2);
    ctx.fill();
  }
  ctx.restore();
}

// Modern Scene Props
function drawSun(x, y, r, isHeat = false) {
  ctx.save();
  // Atmospheric halo
  const haloR = r * (isHeat ? 3.4 : 2.7);
  const halo = ctx.createRadialGradient(x, y, r * 0.4, x, y, haloR);
  halo.addColorStop(0, isHeat ? "rgba(255, 195, 65, 0.72)" : "rgba(255, 235, 130, 0.55)");
  halo.addColorStop(0.5, isHeat ? "rgba(251, 146, 60, 0.22)" : "rgba(255, 215, 90, 0.18)");
  halo.addColorStop(1, "rgba(255, 215, 90, 0)");
  ctx.fillStyle = halo;
  ctx.beginPath();
  ctx.arc(x, y, haloR, 0, PI * 2);
  ctx.fill();

  // Gently rotating sunbeams
  const rot = g.tick * 0.004;
  for (let i = 0; i < 12; i++) {
    const a = rot + i * (PI / 6);
    const inner = r * 1.18;
    const outer = r * (i % 2 === 0 ? 1.62 : 1.42);
    line(
      x + Math.cos(a) * inner,
      y + Math.sin(a) * inner,
      x + Math.cos(a) * outer,
      y + Math.sin(a) * outer,
      "rgba(255, 224, 102, 0.75)",
      3
    );
  }

  // Sun core gradient
  const sunGrad = ctx.createRadialGradient(x - r * 0.25, y - r * 0.25, r * 0.1, x, y, r);
  sunGrad.addColorStop(0, "#fffbeb");
  sunGrad.addColorStop(0.55, "#fde047");
  sunGrad.addColorStop(1, isHeat ? "#f97316" : "#f59e0b");
  ellipseFill(x, y, r, r, sunGrad);
  ctx.restore();
}

function drawCloud(x, y, s, tint = "light") {
  ctx.save();
  const topCol = tint === "dark" ? "#cbd5e1" : tint === "storm" ? "#94a3b8" : "#ffffff";
  const botCol = tint === "dark" ? "#94a3b8" : tint === "storm" ? "#64748b" : "#e2f1ff";

  ctx.shadowColor = "rgba(15, 23, 42, 0.08)";
  ctx.shadowBlur = 10;
  ctx.shadowOffsetY = 4;

  const grad = ctx.createLinearGradient(x, y, x, y + 50 * s);
  grad.addColorStop(0, topCol);
  grad.addColorStop(1, botCol);

  ctx.fillStyle = grad;
  ctx.beginPath();
  ctx.arc(x + 25 * s, y + 32 * s, 18 * s, 0, PI * 2);
  ctx.arc(x + 48 * s, y + 22 * s, 24 * s, 0, PI * 2);
  ctx.arc(x + 76 * s, y + 29 * s, 19 * s, 0, PI * 2);
  ctx.roundRect(x + 12 * s, y + 26 * s, 78 * s, 23 * s, 12 * s);
  ctx.fill();
  ctx.restore();
}

function drawCandy(x, y, color, scale = 1) {
  ctx.save();
  ctx.translate(x, y);
  ctx.scale(scale, scale);

  // Wooden stick with highlight
  line(0, 26, 0, 94, "#92572b", 6);
  line(-1, 26, -1, 92, "#d49a6a", 2);

  // Soft back shadow
  ellipseFill(0, 4, 34, 34, "rgba(15, 23, 42, 0.10)");

  // Fluffy layered puffs
  const puffLight = adjustColor(color, 48);
  const puffMid = adjustColor(color, 18);
  for (let i = 0; i < 10; i++) {
    const a = i * (PI * 2) / 10 + 0.15;
    const px = Math.cos(a) * 18;
    const py = Math.sin(a) * 20;
    const puffGrad = ctx.createRadialGradient(px - 4, py - 5, 2, px, py, 18);
    puffGrad.addColorStop(0, "#ffffff");
    puffGrad.addColorStop(0.35, puffLight);
    puffGrad.addColorStop(1, i % 2 ? puffMid : color);
    ellipseFill(px, py, 16, 16, puffGrad);
  }

  // Center cloud
  const coreGrad = ctx.createRadialGradient(-7, -9, 3, 0, 0, 25);
  coreGrad.addColorStop(0, "#ffffff");
  coreGrad.addColorStop(0.45, puffLight);
  coreGrad.addColorStop(1, color);
  ellipseFill(0, 0, 23, 23, coreGrad);

  // Subtle sugar highlight arcs
  ctx.strokeStyle = "rgba(255, 255, 255, 0.55)";
  ctx.lineWidth = 2;
  ctx.beginPath();
  ctx.arc(-4, -5, 13, PI * 1.05, PI * 1.75);
  ctx.stroke();

  ctx.restore();
}

function drawPerson(x, y, shirtColor, phase, hasCandy = false, hairStyle = 0) {
  const bob = Math.sin((g.tick + phase) * 0.08) * 3.0;
  const py = y + bob;

  // Ground shadow (stays planted)
  ellipseFill(x, y + 32, 17, 5, "rgba(15, 23, 42, 0.16)");

  // Legs & shoes
  line(x - 6, py - 2, x - 8, py + 28, "#1e3a5f", 6);
  line(x + 6, py - 2, x + 8, py + 28, "#1e3a5f", 6);
  ellipseFill(x - 9, py + 30, 5, 3, "#0f172a");
  ellipseFill(x + 9, py + 30, 5, 3, "#0f172a");

  // Torso with shading
  fillGradRound(x - 16, py - 46, 32, 46, 11, adjustColor(shirtColor, 20), adjustColor(shirtColor, -22));

  // Arms
  line(x - 14, py - 38, x - 21, py - 14, adjustColor(shirtColor, -10), 6);
  line(x + 14, py - 38, x + 22, py - (hasCandy ? 28 : 14), adjustColor(shirtColor, -10), 6);
  ellipseFill(x - 21, py - 13, 4, 4, "#f1c29b");
  ellipseFill(x + 22, py - (hasCandy ? 29 : 13), 4, 4, "#f1c29b");

  // Optional mini candyfloss treat
  if (hasCandy) {
    line(x + 23, py - 30, x + 26, py - 44, "#92572b", 2.5);
    ellipseFill(x + 27, py - 48, 8, 9, "#ff85c0");
    ellipseFill(x + 25, py - 50, 4, 4, "rgba(255,255,255,0.65)");
  }

  // Head
  ellipseFill(x, py - 60, 13.5, 14, "#f3c59f");
  // Hair
  const hairCols = ["#4a2c1d", "#1e293b", "#92400e"];
  const hCol = hairCols[hairStyle % hairCols.length];
  ellipseFill(x, py - 68, 14.5, 8.5, hCol);
  if (hairStyle % 2 === 1) {
    ellipseFill(x - 11, py - 60, 4, 9, hCol);
    ellipseFill(x + 11, py - 60, 4, 9, hCol);
  }
  // Eyes & smile
  ellipseFill(x - 4.5, py - 59, 1.5, 1.8, "#1e293b");
  ellipseFill(x + 4.5, py - 59, 1.5, 1.8, "#1e293b");
  ctx.strokeStyle = "#9a3412";
  ctx.lineWidth = 1.5;
  ctx.beginPath();
  ctx.arc(x, py - 55, 4, 0.15 * PI, 0.85 * PI);
  ctx.stroke();
}

function drawSeagull(x, y, s) {
  const flap = Math.sin(g.tick * 0.14 + x * 0.02) * 5 * s;
  ctx.save();
  ctx.strokeStyle = "#ffffff";
  ctx.lineWidth = 2.8 * s;
  ctx.lineCap = "round";
  ctx.beginPath();
  ctx.moveTo(x, y + 8 * s + flap);
  ctx.quadraticCurveTo(x + 10 * s, y - 2 * s, x + 19 * s, y + 7 * s);
  ctx.quadraticCurveTo(x + 28 * s, y - 2 * s, x + 38 * s, y + 8 * s + flap);
  ctx.stroke();
  ctx.restore();
}

function drawFerrisWheel(cx, cy, r) {
  ctx.save();
  ctx.strokeStyle = "rgba(255, 255, 255, 0.35)";
  ctx.lineWidth = 2;
  ctx.beginPath();
  ctx.arc(cx, cy, r, 0, PI * 2);
  ctx.stroke();

  // Support A-frame
  line(cx, cy, cx - r * 0.65, cy + r * 1.25, "rgba(71, 85, 105, 0.55)", 3);
  line(cx, cy, cx + r * 0.65, cy + r * 1.25, "rgba(71, 85, 105, 0.55)", 3);

  const rot = g.tick * 0.005;
  const gondolaCols = ["#fb7185", "#fbbf24", "#38bdf8", "#34d399"];
  for (let i = 0; i < 8; i++) {
    const a = rot + i * (PI / 4);
    const gx = cx + Math.cos(a) * r;
    const gy = cy + Math.sin(a) * r;
    line(cx, cy, gx, gy, "rgba(255, 255, 255, 0.28)", 1.2);
    fillRound(gx - 4, gy, 8, 8, 3, gondolaCols[i % gondolaCols.length]);
  }
  ellipseFill(cx, cy, 5, 5, "#f8fafc");
  ctx.restore();
}

function drawWeatherIconMini(cx, cy, weather) {
  ctx.save();
  if (weather === WEATHER_SUNNY || weather === WEATHER_HEAT) {
    ellipseFill(cx, cy, 8, 8, weather === WEATHER_HEAT ? "#f97316" : "#facc15");
    for (let i = 0; i < 8; i++) {
      const a = i * (PI / 4);
      line(cx + Math.cos(a) * 10, cy + Math.sin(a) * 10, cx + Math.cos(a) * 13, cy + Math.sin(a) * 13, "#f59e0b", 2);
    }
  } else if (weather === WEATHER_BREEZY) {
    ellipseFill(cx - 3, cy - 2, 7, 7, "#facc15");
    ellipseFill(cx + 3, cy + 2, 8, 6, "#cbd5e1");
    line(cx - 10, cy + 9, cx + 8, cy + 9, "#38bdf8", 2);
  } else if (weather === WEATHER_CLOUDY) {
    ellipseFill(cx - 4, cy + 2, 8, 6, "#94a3b8");
    ellipseFill(cx + 3, cy - 1, 9, 7, "#cbd5e1");
    ellipseFill(cx + 8, cy + 3, 6, 5, "#94a3b8");
  } else if (weather === WEATHER_RAIN) {
    ellipseFill(cx, cy - 3, 10, 6, "#64748b");
    line(cx - 5, cy + 5, cx - 7, cy + 11, "#38bdf8", 2);
    line(cx + 1, cy + 5, cx - 1, cy + 11, "#38bdf8", 2);
    line(cx + 7, cy + 5, cx + 5, cy + 11, "#38bdf8", 2);
  } else if (weather === WEATHER_STORM) {
    ellipseFill(cx, cy - 4, 10, 6, "#475569");
    polygonFill([
      { x: cx + 1, y: cy + 1 },
      { x: cx - 4, y: cy + 7 },
      { x: cx, y: cy + 7 },
      { x: cx - 3, y: cy + 13 },
      { x: cx + 5, y: cy + 5 },
      { x: cx + 1, y: cy + 5 }
    ], "#facc15");
  }
  ctx.restore();
}

function drawFlagUK(x, y, w, h) {
  fillRound(x, y, w, h, 3, "rgb(1,33,105)");
  line(x, y, x + w, y + h, "rgb(255,255,255)", 3);
  line(x + w, y, x, y + h, "rgb(255,255,255)", 3);
  line(x, y, x + w, y + h, "rgb(200,16,46)", 1);
  line(x + w, y, x, y + h, "rgb(200,16,46)", 1);
  const cx = x + w / 2, cy = y + h / 2;
  line(cx, y, cx, y + h, "rgb(255,255,255)", 5);
  line(x, cy, x + w, cy, "rgb(255,255,255)", 5);
  line(cx, y, cx, y + h, "rgb(200,16,46)", 2);
  line(x, cy, x + w, cy, "rgb(200,16,46)", 2);
  strokeRound(x, y, w, h, 3, "rgba(15, 23, 42, 0.35)", 1);
}

function drawFlagKO(x, y, w, h) {
  fillRound(x, y, w, h, 3, "rgb(255,255,255)");
  const cx = x + w / 2, cy = y + h / 2, cr = h / 3;
  ellipseFill(cx, cy, cr, cr, "rgb(5,75,150)");

  ctx.fillStyle = "rgb(205,46,58)";
  ctx.beginPath();
  ctx.arc(cx, cy, cr, 0, PI, true);
  ctx.fill();

  ellipseFill(cx - cr / 2, cy, cr / 2, cr / 2, "rgb(205,46,58)");
  ellipseFill(cx + cr / 2, cy, cr / 2, cr / 2, "rgb(5,75,150)");

  const blk = "rgb(30,30,30)";
  line(x + 2, y + 2, x + 6, y + 2, blk, 1);
  line(x + 2, y + 4, x + 6, y + 4, blk, 1);
  line(x + 2, y + 6, x + 6, y + 6, blk, 1);
  line(x + w - 6, y + h - 6, x + w - 2, y + h - 6, blk, 1);
  line(x + w - 6, y + h - 4, x + w - 2, y + h - 4, blk, 1);
  line(x + w - 6, y + h - 2, x + w - 2, y + h - 2, blk, 1);
  strokeRound(x, y, w, h, 3, "rgba(15, 23, 42, 0.35)", 1);
}

// Coastal Scene Drawing (Left 640px on Plan Screen)
function drawScene() {
  // 1. Sky Gradient
  let skyTop = "#38bdf8", skyBot = "#bae6fd";
  if (g.weather === WEATHER_STORM) {
    skyTop = "#334155"; skyBot = "#64748b";
  } else if (g.weather === WEATHER_RAIN) {
    skyTop = "#64748b"; skyBot = "#cbd5e1";
  } else if (g.weather === WEATHER_CLOUDY) {
    skyTop = "#7dd3fc"; skyBot = "#e2e8f0";
  } else if (g.weather === WEATHER_HEAT) {
    skyTop = "#0284c7"; skyBot = "#fde68a";
  }
  const skyGrad = ctx.createLinearGradient(0, 0, 0, 290);
  skyGrad.addColorStop(0, skyTop);
  skyGrad.addColorStop(1, skyBot);
  ctx.fillStyle = skyGrad;
  ctx.fillRect(0, 0, 640, 290);

  // Occasional soft lightning flash in storm
  if (g.weather === WEATHER_STORM && (g.tick % 210) < 4) {
    ctx.fillStyle = "rgba(255, 255, 255, 0.22)";
    ctx.fillRect(0, 0, 640, 290);
  }

  // Sun / Clouds
  if (g.weather === WEATHER_SUNNY || g.weather === WEATHER_BREEZY || g.weather === WEATHER_HEAT) {
    drawSun(545, 76, g.weather === WEATHER_HEAT ? 36 : 28, g.weather === WEATHER_HEAT);
  }

  const cloudTint = g.weather === WEATHER_STORM ? "storm" : (g.weather === WEATHER_RAIN || g.weather === WEATHER_CLOUDY ? "dark" : "light");
  drawCloud(50 + Math.sin(g.tick * 0.013) * 14, 52, 0.95, cloudTint);
  drawCloud(285 + Math.sin(g.tick * 0.010) * 18, 34, 1.05, cloudTint);
  if (g.weather === WEATHER_CLOUDY || g.weather === WEATHER_RAIN || g.weather === WEATHER_STORM) {
    drawCloud(470 + Math.cos(g.tick * 0.012) * 14, 60, 1.0, cloudTint);
  }

  // Distant coastline hills & Ferris wheel
  ctx.fillStyle = g.weather === WEATHER_STORM ? "rgba(51, 65, 85, 0.45)" : "rgba(14, 116, 144, 0.22)";
  ctx.beginPath();
  ctx.moveTo(0, 280);
  ctx.quadraticCurveTo(95, 244, 210, 276);
  ctx.quadraticCurveTo(320, 255, 420, 280);
  ctx.lineTo(0, 280);
  ctx.fill();
  drawFerrisWheel(378, 224, 34);

  // 2. Ocean Gradient + Shimmer + Waves
  const seaGrad = ctx.createLinearGradient(0, 280, 0, 396);
  seaGrad.addColorStop(0, g.weather === WEATHER_STORM ? "#1e3a5f" : "#0284c7");
  seaGrad.addColorStop(0.75, g.weather === WEATHER_STORM ? "#334155" : "#0ea5e9");
  seaGrad.addColorStop(1, "#7dd3fc");
  ctx.fillStyle = seaGrad;
  ctx.fillRect(0, 280, 640, 116);

  // Sun reflection on water
  if (g.weather === WEATHER_SUNNY || g.weather === WEATHER_BREEZY || g.weather === WEATHER_HEAT) {
    for (let i = 0; i < 6; i++) {
      const rw = 44 - i * 5;
      const rx = 540 + Math.sin(g.tick * 0.06 + i) * 6 - rw / 2;
      fillRound(rx, 288 + i * 14, rw, 4, 2, "rgba(254, 240, 138, 0.42)");
    }
  }

  // Animated wave crests
  for (let i = 0; i < 9; i++) {
    const xx = (g.tick * 1.6 + i * 95) % 740 - 60;
    const yy = 310 + (i % 3) * 20;
    line(xx, yy, xx + 38, yy + Math.sin(g.tick * 0.08 + i) * 2.5, "rgba(224, 242, 254, 0.65)", 2.5);
  }

  // 3. Sandy Beach + Wet Shoreline Foam
  const sandGrad = ctx.createLinearGradient(0, 386, 0, 600);
  sandGrad.addColorStop(0, "#d4b483");
  sandGrad.addColorStop(0.14, "#f4d7a1");
  sandGrad.addColorStop(1, "#e7be7d");
  ctx.fillStyle = sandGrad;
  ctx.fillRect(0, 386, 640, 214);

  // Shoreline foam
  const tideOffset = Math.sin(g.tick * 0.04) * 4;
  ctx.strokeStyle = "rgba(255, 255, 255, 0.78)";
  ctx.lineWidth = 4;
  ctx.beginPath();
  ctx.moveTo(0, 390 + tideOffset);
  for (let x = 0; x <= 640; x += 40) {
    ctx.quadraticCurveTo(x + 20, 385 + tideOffset + Math.sin(x * 0.04 + g.tick * 0.05) * 4, x + 40, 390 + tideOffset);
  }
  ctx.stroke();

  // Subtle beach pebbles
  const pebbleCols = ["rgba(180, 135, 80, 0.35)", "rgba(255, 248, 225, 0.45)"];
  for (let i = 0; i < 12; i++) {
    ellipseFill(45 + (i * 53) % 580, 480 + (i * 29) % 95, 3 + (i % 3), 2, pebbleCols[i % 2]);
  }

  // 4. Seaside Pier
  // Water shadow under pier
  fillRound(394, 244, 232, 16, 4, "rgba(15, 23, 42, 0.2)");
  for (let x = 408; x < 625; x += 42) {
    line(x, 244, x, 348, "#475569", 6);
    line(x + 1.5, 244, x + 1.5, 348, "#64748b", 2);
  }
  // Pier deck & railing
  fillGradRound(386, 228, 244, 16, 4, "#64748b", "#475569");
  line(392, 218, 624, 218, "rgba(241, 245, 249, 0.85)", 2);
  for (let x = 398; x <= 622; x += 28) {
    line(x, 218, x, 228, "rgba(226, 232, 240, 0.85)", 2);
  }

  // Pier pavilion
  ctx.save();
  ctx.shadowColor = "rgba(15, 23, 42, 0.22)";
  ctx.shadowBlur = 12;
  ctx.shadowOffsetY = 5;
  fillGradRound(456, 178, 154, 52, 8, "#ffffff", "#f1f5f9");
  ctx.restore();
  fillGradRound(446, 164, 174, 18, 8, "#f43f5e", "#be123c");
  centreText(456, 182, 610, 206, locationsData[g.loc].pierName, "#1e3a8a", "bold 14px Outfit, sans-serif");
  const zName = g.lang === LANG_EN ? beachZoneNamesEN[g.currentPlayer] : beachZoneNamesKO[g.currentPlayer];
  centreText(456, 204, 610, 225, zName, "#64748b", "600 12px Outfit, sans-serif");

  // 5. Dynamic Advertising Signs (based on p.adverts)
  const p = g.players[g.currentPlayer];
  const sc = stallColours[p.stallColour];
  const scDark = stallDarkColours[p.stallColour];
  const shownAds = Math.min(p.adverts, 5);
  for (let i = 0; i < shownAds; i++) {
    const ax = 365 + i * 32;
    const ay = 394 + (i % 2) * 14;
    line(ax + 8, ay, ax + 4, ay + 24, "#78350f", 2.5);
    line(ax + 12, ay, ax + 16, ay + 24, "#78350f", 2.5);
    fillGradRound(ax, ay - 12, 20, 16, 4, sc, scDark);
    fillRound(ax + 3, ay - 9, 14, 10, 2, "#ffffff");
    ellipseFill(ax + 10, ay - 4, 3.5, 3.5, sc);
  }

  // 6. Candyfloss Stall
  // Ground shadow
  ellipseFill(192, 466, 162, 16, "rgba(15, 23, 42, 0.18)");

  // Stall Body
  ctx.save();
  ctx.shadowColor = "rgba(15, 23, 42, 0.16)";
  ctx.shadowBlur = 14;
  ctx.shadowOffsetY = 6;
  fillGradRound(48, 236, 288, 226, 14, "#ffffff", adjustColor(sc, 75));
  ctx.restore();

  // Vertical decorative stripes on stall front
  for (let i = 0; i < 6; i++) {
    fillRound(56 + i * 46, 408, 23, 50, 4, i % 2 === 0 ? "rgba(255,255,255,0.65)" : adjustColor(sc, 35));
  }

  // Seller behind counter
  const sellerBob = Math.sin(g.tick * 0.08) * 2.5;
  fillGradRound(108, 326 + sellerBob, 68, 72, 16, sc, scDark);
  // Apron
  fillRound(120, 346 + sellerBob, 44, 48, 8, "rgba(255, 255, 255, 0.92)");
  ellipseFill(142, 305 + sellerBob, 21, 21, "#f3c59f");
  ellipseFill(142, 289 + sellerBob, 23, 12, "#4a2c1d");
  ellipseFill(135, 304 + sellerBob, 2.2, 2.5, "#1e293b");
  ellipseFill(149, 304 + sellerBob, 2.2, 2.5, "#1e293b");
  ctx.strokeStyle = "#9a3412";
  ctx.lineWidth = 2;
  ctx.beginPath();
  ctx.arc(142, 310 + sellerBob, 5.5, 0.15 * PI, 0.85 * PI);
  ctx.stroke();

  // Spinning Candyfloss Machine on Counter
  fillGradRound(226, 362, 68, 30, 8, "#cbd5e1", "#94a3b8");
  ellipseFill(260, 360, 56, 22, "#e2e8f0");
  ellipseFill(260, 358, 48, 17, "#94a3b8");
  ellipseFill(260, 357, 36, 12, "#ff85c0");
  // Subtle swirl animation inside machine
  const swAng = g.tick * 0.25;
  line(260 + Math.cos(swAng) * 18, 357 + Math.sin(swAng) * 5, 260 - Math.cos(swAng) * 18, 357 - Math.sin(swAng) * 5, "rgba(255,255,255,0.7)", 2);

  // Display Candyfloss Stick
  drawCandy(194, 286, sc, 0.88);

  // Wooden Countertop
  fillGradRound(34, 388, 316, 24, 10, "#92572b", "#6b3e1d");
  line(42, 391, 342, 391, "rgba(255, 230, 200, 0.35)", 2);

  // Striped 3D Scalloped Awning
  for (let i = 0; i < 6; i++) {
    const ax = 36 + i * 52;
    const isStripe = i % 2 === 0;
    fillGradRound(ax, 202, 52, 42, 0, isStripe ? sc : "#fff8f1", isStripe ? scDark : "#f1e4d6");
    ellipseFill(ax + 26, 244, 26, 10, isStripe ? scDark : "#f1e4d6");
  }

  // Stall Signboard
  ctx.save();
  ctx.shadowColor = "rgba(15, 23, 42, 0.22)";
  ctx.shadowBlur = 12;
  ctx.shadowOffsetY = 4;
  fillRound(68, 140, 248, 62, 18, "#ffffff");
  ctx.restore();
  strokeRound(68, 140, 248, 62, 18, sc, 2.5);
  centreText(74, 145, 310, 175, p.businessName, scDark, "800 19px Outfit, sans-serif");
  centreText(74, 173, 310, 196, g.lang === LANG_EN ? "CANDYFLOSS STALL" : "솜사탕 가게", "#475569", "700 12px Outfit, sans-serif");

  // 7. Customers
  drawPerson(412, 444, "rgb(56, 189, 248)", 0, true, 0);
  drawPerson(484, 456, "rgb(244, 114, 182)", 18, false, 1);
  drawPerson(556, 438, "rgb(52, 211, 153)", 34, true, 2);

  // 8. Swaying Bunting & Seagulls
  const flags = ["#f43f5e", "#facc15", "#38bdf8", "#34d399"];
  ctx.strokeStyle = "rgba(255, 255, 255, 0.85)";
  ctx.lineWidth = 2;
  ctx.beginPath();
  ctx.moveTo(10, 116);
  ctx.quadraticCurveTo(320, 148, 632, 134);
  ctx.stroke();

  for (let i = 0; i < 13; i++) {
    const t = (i + 0.5) / 13;
    const bx = 10 + t * 622;
    const by = (1 - t) * (1 - t) * 116 + 2 * (1 - t) * t * 148 + t * t * 134;
    const sway = Math.sin(g.tick * 0.06 + i) * 2;
    polygonFill(
      [{ x: bx - 9, y: by }, { x: bx + 9, y: by }, { x: bx + sway, y: by + 20 }],
      flags[i % 4]
    );
  }
  drawSeagull(20 + (g.tick * 1.8) % 680, 74 + Math.sin(g.tick * 0.04) * 6, 0.9);
  drawSeagull(340 + (g.tick * 1.3) % 680 - 300, 102 + Math.cos(g.tick * 0.05) * 5, 0.72);

  // 9. Weather Particles
  if (g.weather === WEATHER_RAIN || g.weather === WEATHER_STORM) {
    const count = g.weather === WEATHER_STORM ? 55 : 28;
    for (let i = 0; i < count; i++) {
      const rx = (i * 73 + g.tick * 6) % 640;
      const ry = (i * 47 + g.tick * 11) % 520;
      line(rx, ry, rx - 8, ry + 18, "rgba(224, 242, 254, 0.75)", 1.8);
    }
  } else if (g.weather === WEATHER_BREEZY) {
    for (let i = 0; i < 4; i++) {
      const wx = (g.tick * 3 + i * 180) % 720 - 60;
      const wy = 160 + i * 55;
      line(wx, wy, wx + 46, wy - 4, "rgba(255, 255, 255, 0.35)", 2);
    }
  }
}

// UI Buttons & Sliders Drawing
function drawButton(b, color, enabled = true) {
  const w = b.r.right - b.r.left;
  const h = b.r.bottom - b.r.top;
  const hovered = enabled && isHover(b.r);
  const lift = hovered ? -2 : 0;

  ctx.save();
  ctx.shadowColor = enabled ? "rgba(15, 23, 42, 0.26)" : "rgba(15, 23, 42, 0.08)";
  ctx.shadowBlur = hovered ? 18 : 10;
  ctx.shadowOffsetY = hovered ? 7 : 4;

  const topCol = enabled ? adjustColor(color, hovered ? 22 : 14) : "#cbd5e1";
  const botCol = enabled ? adjustColor(color, hovered ? -8 : -16) : "#94a3b8";
  fillGradRound(b.r.left, b.r.top + lift, w, h, Math.min(24, h / 2), topCol, botCol);
  ctx.restore();

  // Subtle top glossy highlight
  fillRound(b.r.left + 6, b.r.top + 3 + lift, w - 12, Math.max(4, h * 0.28), Math.min(18, h / 3), "rgba(255, 255, 255, 0.24)");
  strokeRound(b.r.left, b.r.top + lift, w, h, Math.min(24, h / 2), hovered ? "rgba(255, 255, 255, 0.9)" : "rgba(255, 255, 255, 0.45)", 1.5);
  centreText(b.r.left, b.r.top + lift, b.r.right, b.r.bottom + lift, b.text, "#ffffff", "700 17px Outfit, sans-serif");
}

function drawSlider(s, label, valueStr) {
  textLeft(s.r.left, s.r.top - 28, label, "#1e293b", "700 15px Outfit, sans-serif");

  // Value pill
  fillRound(s.r.right - 98, s.r.top - 32, 98, 24, 12, "#fdf2f8");
  strokeRound(s.r.right - 98, s.r.top - 32, 98, 24, 12, "#fbcfe8", 1);
  centreText(s.r.right - 98, s.r.top - 32, s.r.right, s.r.top - 8, valueStr, "#be185d", "700 14px Outfit, sans-serif");

  const cy = (s.r.top + s.r.bottom) / 2;
  const pct = (s.value - s.minv) / (s.maxv - s.minv);
  const kx = s.r.left + Math.floor(pct * (s.r.right - s.r.left));

  // Track
  line(s.r.left, cy, s.r.right, cy, "#e2e8f0", 10);
  const fillGrad = ctx.createLinearGradient(s.r.left, cy, kx, cy);
  fillGrad.addColorStop(0, "#fb7185");
  fillGrad.addColorStop(1, "#ec4899");
  if (kx > s.r.left) {
    line(s.r.left, cy, kx, cy, fillGrad, 10);
  }

  // Knob
  const hovered = isHover(s.r) || g.activeSlider === (s.id - 1);
  ctx.save();
  ctx.shadowColor = "rgba(236, 72, 153, 0.45)";
  ctx.shadowBlur = hovered ? 12 : 6;
  ellipseFill(kx, cy, hovered ? 15 : 13, hovered ? 15 : 13, "#ffffff");
  ctx.restore();
  ellipseFill(kx, cy, 7.5, 7.5, "#ec4899");
}

// Screen 0: Title Screen
function drawTitle() {
  // Coastal sky gradient
  const sky = ctx.createLinearGradient(0, 0, 0, 420);
  sky.addColorStop(0, "#0284c7");
  sky.addColorStop(0.6, "#38bdf8");
  sky.addColorStop(1, "#bae6fd");
  ctx.fillStyle = sky;
  ctx.fillRect(0, 0, APP_W, 420);

  drawSun(845, 86, 38, false);
  drawCloud(55 + Math.sin(g.tick * 0.014) * 16, 46, 1.05);
  drawCloud(640 + Math.sin(g.tick * 0.010) * 20, 32, 0.95);
  drawCloud(355 + Math.sin(g.tick * 0.012) * 12, 62, 0.85);

  // Distant Ferris Wheel & Pier Silhouette
  drawFerrisWheel(130, 368, 48);

  // Ocean
  const sea = ctx.createLinearGradient(0, 415, 0, 505);
  sea.addColorStop(0, "#0369a1");
  sea.addColorStop(1, "#38bdf8");
  ctx.fillStyle = sea;
  ctx.fillRect(0, 415, APP_W, 90);

  for (let i = 0; i < 11; i++) {
    const xx = (g.tick * 2 + i * 110) % 1080 - 60;
    line(xx, 448 + (i % 2) * 16, xx + 50, 448 + (i % 2) * 16 + Math.sin(g.tick * 0.07 + i) * 2, "rgba(224, 242, 254, 0.7)", 2.5);
  }

  // Sandy Foreground
  const sand = ctx.createLinearGradient(0, 500, 0, 600);
  sand.addColorStop(0, "#f5deb3");
  sand.addColorStop(1, "#deb887");
  ctx.fillStyle = sand;
  ctx.fillRect(0, 500, APP_W, 100);

  // Festive Bunting
  line(0, 14, APP_W, 28, "rgba(255, 255, 255, 0.85)", 2);
  const flags = ["#f43f5e", "#facc15", "#38bdf8", "#34d399"];
  for (let i = 0; i < 20; i++) {
    const x = 10 + i * 48, y = 14 + (i % 2) * 4;
    polygonFill([{ x, y }, { x: x + 20, y: y + 1 }, { x: x + 10, y: y + 24 }], flags[i % 4]);
  }

  drawSeagull(220 + (g.tick * 1.6) % 720, 92 + Math.sin(g.tick * 0.04) * 5, 0.9);

  // Feature Candyfloss Illustrations
  drawCandy(145, 225 + Math.sin(g.tick * 0.04) * 6, "rgb(244, 63, 142)", 1.15);
  drawCandy(815, 225 + Math.cos(g.tick * 0.04) * 6, "rgb(37, 150, 235)", 1.15);

  // Title Typography
  const t1 = g.lang === LANG_EN ? "CANDYFLOSS" : "솜사탕";
  const t2 = g.lang === LANG_EN ? "TYCOON" : "타이쿤";
  const sub = g.lang === LANG_EN ? "★ 1-4 PLAYER FLOPPY EDITION ★" : "★ 1-4인용 플로피 에디션 ★";

  ctx.save();
  ctx.shadowColor = "rgba(15, 23, 42, 0.32)";
  ctx.shadowBlur = 16;
  ctx.shadowOffsetY = 5;
  centreText(130, 32, 830, 98, t1, "#ffffff", "900 54px Outfit, sans-serif");
  centreText(250, 92, 710, 148, t2, "#fde047", "900 36px Outfit, sans-serif");
  ctx.restore();

  fillRound(290, 154, 380, 36, 18, "rgba(15, 23, 42, 0.82)");
  strokeRound(290, 154, 380, 36, 18, "rgba(255, 255, 255, 0.25)", 1);
  centreText(290, 154, 670, 190, sub, "#fde047", "700 15px Outfit, sans-serif");

  // Frosted Glass Menu Panel
  ctx.save();
  ctx.shadowColor = "rgba(15, 23, 42, 0.22)";
  ctx.shadowBlur = 24;
  ctx.shadowOffsetY = 10;
  fillRound(320, 204, 320, 318, 28, "rgba(255, 255, 255, 0.30)");
  ctx.restore();
  strokeRound(320, 204, 320, 318, 28, "rgba(255, 255, 255, 0.55)", 1.5);

  titleButtons[0].text = g.lang === LANG_EN ? "PLAY GAME" : "게임 시작";
  titleButtons[1].text = g.lang === LANG_EN ? "HOW TO PLAY" : "게임 설명";
  titleButtons[2].text = g.lang === LANG_EN ? "HALL OF FAME" : "명예의 전당";
  titleButtons[3].text = g.soundOn ? (g.lang === LANG_EN ? "SOUND: ON" : "소리: 켜짐") : (g.lang === LANG_EN ? "SOUND: OFF" : "소리: 꺼짐");
  titleButtons[4].text = g.lang === LANG_EN ? "English / 한국어" : "한국어 / English";

  const btnCols = [
    "rgb(244, 63, 142)",
    "rgb(37, 150, 235)",
    "rgb(245, 158, 11)",
    g.soundOn ? "rgb(16, 185, 129)" : "rgb(249, 115, 22)",
    "rgb(139, 92, 246)"
  ];

  for (let i = 0; i < 5; i++) {
    drawButton(titleButtons[i], btnCols[i]);
    if (i === 4) {
      const lift = isHover(titleButtons[4].r) ? -2 : 0;
      drawFlagUK(366, 475 + lift, 28, 18);
      drawFlagKO(566, 475 + lift, 28, 18);
      strokeRound(g.lang === LANG_EN ? 364 : 564, 473 + lift, 32, 22, 4, "#ffffff", 2);
    }
  }

  // Clickable 1982 BBC Micro Inspiration Pill
  const inspHovered = isHover(titleInspirationBtn.r);
  const ir = titleInspirationBtn.r;
  fillRound(ir.left, ir.top, ir.right - ir.left, ir.bottom - ir.top, 19, inspHovered ? "rgba(15, 23, 42, 0.94)" : "rgba(15, 23, 42, 0.80)");
  strokeRound(ir.left, ir.top, ir.right - ir.left, ir.bottom - ir.top, 19, inspHovered ? "#fde047" : "rgba(253, 224, 71, 0.45)", 1.5);
  const inspText = g.lang === LANG_EN
    ? "🕹️ Inspired by 1982 BBC Micro Candyfloss  •  View Original ↗"
    : "🕹️ 1982년 BBC Micro 'Candyfloss' 원작 오마주  •  원작 보기 ↗";
  centreText(ir.left, ir.top, ir.right, ir.bottom, inspText, "#fde047", "700 13.5px Outfit, sans-serif");
}

// Screen 1: Game Setup
function drawSetup() {
  drawModernBackdrop("#e0f2fe", "#f0f9ff", "#fdf2f8");
  centreText(160, 18, 800, 62, g.lang === LANG_EN ? "GAME SETUP" : "게임 설정", "#0f172a", "800 32px Outfit, sans-serif");

  // Location Selector
  textLeft(60, 74, g.lang === LANG_EN ? "Seaside Location" : "해변 지역 선택", "#1e293b", "700 15px Outfit, sans-serif");
  for (let i = 0; i < 7; i++) {
    const r = { left: 60 + i * 122, top: 102, right: 175 + i * 122, bottom: 146 };
    const selected = g.loc === i;
    const hovered = isHover(r);
    if (selected) {
      fillGradRound(r.left, r.top, 115, 44, 14, "#f43f8e", "#db2777");
    } else {
      fillRound(r.left, r.top, 115, 44, 14, hovered ? "#ffffff" : "rgba(255,255,255,0.82)");
      strokeRound(r.left, r.top, 115, 44, 14, hovered ? "#f43f8e" : "#cbd5e1", 1.5);
    }
    const lName = g.lang === LANG_EN ? locationsData[i].nameEN : locationsData[i].nameKO;
    centreText(r.left, r.top, r.right, r.bottom - 10, lName, selected ? "#ffffff" : "#1e293b", "700 13px Outfit, sans-serif");
    centreText(r.left, r.top + 22, r.right, r.bottom - 2, locationsData[i].country, selected ? "rgba(255,255,255,0.85)" : "#64748b", "700 10px Outfit, sans-serif");
  }

  // Player Count
  textLeft(60, 162, g.lang === LANG_EN ? "Number of Players" : "플레이어 수", "#1e293b", "700 15px Outfit, sans-serif");
  const pCountEN = ["1 PLAYER", "2 PLAYERS", "3 PLAYERS", "4 PLAYERS"];
  const pCountKO = ["1인용", "2인용", "3인용", "4인용"];
  for (let i = 0; i < 4; i++) {
    const r = { left: 60 + i * 140, top: 190, right: 185 + i * 140, bottom: 232 };
    const selected = g.numPlayers === (i + 1);
    const hovered = isHover(r);
    if (selected) {
      fillGradRound(r.left, r.top, 125, 42, 14, "#f43f8e", "#db2777");
    } else {
      fillRound(r.left, r.top, 125, 42, 14, hovered ? "#ffffff" : "rgba(255,255,255,0.82)");
      strokeRound(r.left, r.top, 125, 42, 14, hovered ? "#f43f8e" : "#cbd5e1", 1.5);
    }
    centreText(r.left, r.top, r.right, r.bottom, g.lang === LANG_EN ? pCountEN[i] : pCountKO[i], selected ? "#ffffff" : "#1e293b", "700 13.5px Outfit, sans-serif");
  }

  // Days
  textLeft(630, 162, g.lang === LANG_EN ? "Season Length" : "게임 기간", "#1e293b", "700 15px Outfit, sans-serif");
  const daysEN = ["7 DAYS", "10 DAYS", "14 DAYS"];
  const daysKO = ["7일간", "10일간", "14일간"];
  for (let i = 0; i < 3; i++) {
    const targetDays = i === 0 ? 7 : i === 1 ? 10 : 14;
    const r = { left: 630 + i * 100, top: 190, right: 720 + i * 100, bottom: 232 };
    const selected = g.totalDays === targetDays;
    const hovered = isHover(r);
    if (selected) {
      fillGradRound(r.left, r.top, 90, 42, 14, "#38bdf8", "#0284c7");
    } else {
      fillRound(r.left, r.top, 90, 42, 14, hovered ? "#ffffff" : "rgba(255,255,255,0.82)");
      strokeRound(r.left, r.top, 90, 42, 14, hovered ? "#0284c7" : "#cbd5e1", 1.5);
    }
    centreText(r.left, r.top, r.right, r.bottom, g.lang === LANG_EN ? daysEN[i] : daysKO[i], selected ? "#ffffff" : "#1e293b", "700 13.5px Outfit, sans-serif");
  }

  // Difficulty
  textLeft(630, 248, g.lang === LANG_EN ? "Difficulty" : "난이도", "#1e293b", "700 15px Outfit, sans-serif");
  const diffEN = ["EASY", "NORMAL", "HARD"];
  const diffKO = ["쉬움", "보통", "어려움"];
  for (let i = 0; i < 3; i++) {
    const r = { left: 630 + i * 100, top: 274, right: 720 + i * 100, bottom: 314 };
    const selected = g.difficulty === i;
    const hovered = isHover(r);
    if (selected) {
      fillGradRound(r.left, r.top, 90, 40, 14, "#10b981", "#059669");
    } else {
      fillRound(r.left, r.top, 90, 40, 14, hovered ? "#ffffff" : "rgba(255,255,255,0.82)");
      strokeRound(r.left, r.top, 90, 40, 14, hovered ? "#10b981" : "#cbd5e1", 1.5);
    }
    centreText(r.left, r.top, r.right, r.bottom, g.lang === LANG_EN ? diffEN[i] : diffKO[i], selected ? "#ffffff" : "#1e293b", "700 13px Outfit, sans-serif");
  }

  // Business Name Editing Boxes
  textLeft(60, 248, g.lang === LANG_EN ? "Business Names (Click box to edit)" : "가게 상호명 (상자를 클릭하여 입력)", "#1e293b", "700 15px Outfit, sans-serif");
  for (let i = 0; i < g.numPlayers; i++) {
    const boxR = { left: 60, top: 280 + i * 52, right: 500, bottom: 322 + i * 52 };
    const hoveredBox = isHover(boxR);
    fillRound(boxR.left, boxR.top, 440, 42, 12, g.editingNamePlayer === i ? "#fef3c7" : "#ffffff");
    strokeRound(boxR.left, boxR.top, 440, 42, 12, hoveredBox ? stallColours[g.players[i].stallColour] : "#cbd5e1", hoveredBox ? 2 : 1.5);

    const pName = g.lang === LANG_EN ? playerNamesEN[i] : playerNamesKO[i];
    textLeft(76, 291 + i * 52, `${pName}:`, stallColours[g.players[i].stallColour], "700 15px Outfit, sans-serif");
    textLeft(182, 291 + i * 52, `${g.players[i].businessName}${g.editingNamePlayer === i ? '|' : ''}`, "#1e293b", "700 15px Outfit, sans-serif");

    // Preset button
    const preR = { left: 512, top: 280 + i * 52, right: 606, bottom: 322 + i * 52 };
    const hoveredPre = isHover(preR);
    fillRound(preR.left, preR.top, 94, 42, 12, hoveredPre ? "#e0f2fe" : "#ffffff");
    strokeRound(preR.left, preR.top, 94, 42, 12, hoveredPre ? "#0284c7" : "#cbd5e1", 1.5);
    centreText(preR.left, preR.top, preR.right, preR.bottom, g.lang === LANG_EN ? "PRESET" : "추천", "#0284c7", "700 13px Outfit, sans-serif");
  }

  if (g.numPlayers === 1) {
    textLeft(630, 336, g.lang === LANG_EN ? "Stall Colour" : "점포 색상", "#1e293b", "700 15px Outfit, sans-serif");
    for (let i = 0; i < 4; i++) {
      const r = { left: 630 + i * 74, top: 364, right: 694 + i * 74, bottom: 406 };
      fillGradRound(r.left, r.top, 64, 42, 14, stallColours[i], stallDarkColours[i]);
      if (g.players[0].stallColour === i) {
        strokeRound(r.left - 2, r.top - 2, 68, 46, 16, "#0f172a", 2.5);
      } else if (isHover(r)) {
        strokeRound(r.left, r.top, 64, 42, 14, "#ffffff", 2);
      }
    }
  }

  setupButtons[0].text = g.lang === LANG_EN ? "START GAME" : "게임 시작";
  setupButtons[1].text = g.lang === LANG_EN ? "BACK" : "뒤로";
  drawButton(setupButtons[0], "rgb(244, 63, 142)");
  drawButton(setupButtons[1], "rgb(37, 150, 235)");
}

// Screen 2: Pass Turn
function drawPassTurn() {
  drawModernBackdrop("#0f172a", "#0369a1", "#38bdf8");
  drawCardPanel(150, 74, 660, 444, 30);

  const zName = g.lang === LANG_EN ? beachZoneNamesEN[g.currentPlayer] : beachZoneNamesKO[g.currentPlayer];
  const locName = g.lang === LANG_EN ? locationsData[g.loc].nameEN : locationsData[g.loc].nameKO;
  const headerStr = g.lang === LANG_EN ? `DAY ${g.day} / ${g.totalDays}  •  ${locName} (${zName})` : `${g.day}일차 / 총 ${g.totalDays}일  •  ${locName} (${zName})`;
  centreText(200, 98, 760, 134, headerStr, "#64748b", "700 17px Outfit, sans-serif");

  const pName = g.lang === LANG_EN ? playerNamesEN[g.currentPlayer] : playerNamesKO[g.currentPlayer];
  const passStr = g.lang === LANG_EN ? `PASS CONTROL TO ${pName.toUpperCase()}` : `${pName} 차례입니다`;
  centreText(180, 142, 780, 200, passStr, stallColours[g.players[g.currentPlayer].stallColour], "800 30px Outfit, sans-serif");

  centreText(180, 206, 780, 248, g.players[g.currentPlayer].businessName, "#0f172a", "800 26px Outfit, sans-serif");

  fillRound(210, 264, 540, 50, 16, "#f8fafc");
  strokeRound(210, 264, 540, 50, 16, "#e2e8f0", 1);
  const mStr = moneyStr(g.players[g.currentPlayer].cashPence);
  const balStr = g.lang === LANG_EN ? `Current Balance: ${mStr}   •   Reputation: ${g.players[g.currentPlayer].reputation}` : `현재 잔액: ${mStr}   •   평판: ${g.players[g.currentPlayer].reputation}점`;
  centreText(210, 264, 750, 314, balStr, "#334155", "700 18px Outfit, sans-serif");

  const wName = g.lang === LANG_EN ? weatherNamesEN[g.weather] : weatherNamesKO[g.weather];
  const wStr = g.lang === LANG_EN ? `Weather Today: ${wName}  (Expected Crowd: ${g.crowd})` : `오늘 날씨: ${wName}  (유동인구: ${g.crowd}명)`;
  centreText(180, 336, 780, 376, wStr, "#64748b", "600 16px Outfit, sans-serif");

  passButtons[0].text = g.lang === LANG_EN ? "START FLOSS SPINNING!" : "솜사탕 제조 시작!";
  drawButton(passButtons[0], stallColours[g.players[g.currentPlayer].stallColour]);
}

// Screen 3: Arcade Mini-Game
function drawSpinningMiniGame() {
  drawModernBackdrop("#0f172a", "#1e1b4b", "#312e81");

  const cx = 480, cy = 292;
  const spot = ctx.createRadialGradient(cx, cy, 40, cx, cy, 320);
  spot.addColorStop(0, "rgba(244, 63, 142, 0.22)");
  spot.addColorStop(0.55, "rgba(56, 189, 248, 0.12)");
  spot.addColorStop(1, "rgba(15, 23, 42, 0)");
  ctx.fillStyle = spot;
  ctx.fillRect(0, 0, APP_W, APP_H);

  const elapsed = Date.now() - g.spinStartTick;
  const remainingMs = Math.max(0, 5000 - elapsed);
  const progress = remainingMs / 5000;

  const pName = g.lang === LANG_EN ? playerNamesEN[g.currentPlayer] : playerNamesKO[g.currentPlayer];
  centreText(100, 18, 860, 56, `${pName}  •  ${g.players[g.currentPlayer].businessName}`, stallColours[g.players[g.currentPlayer].stallColour], "800 26px Outfit, sans-serif");

  const titleStr = g.lang === LANG_EN ? "SWIRL MOUSE / TOUCH IN CIRCLES TO CRAFT BONUS FLOSS!" : "마우스를 원형으로 빙글빙글 돌려 보너스 솜사탕을 만드세요!";
  centreText(100, 58, 860, 92, titleStr, "#fde047", "700 17px Outfit, sans-serif");

  // Outer timer ring
  ctx.strokeStyle = "rgba(255, 255, 255, 0.12)";
  ctx.lineWidth = 10;
  ctx.beginPath();
  ctx.arc(cx, cy, 186, 0, PI * 2);
  ctx.stroke();

  ctx.strokeStyle = remainingMs < 1500 ? "#fb7185" : "#38bdf8";
  ctx.lineWidth = 10;
  ctx.lineCap = "round";
  ctx.beginPath();
  ctx.arc(cx, cy, 186, -PI / 2, -PI / 2 + PI * 2 * progress);
  ctx.stroke();

  // Stainless steel bowl
  const outerBowl = ctx.createRadialGradient(cx - 40, cy - 40, 20, cx, cy, 170);
  outerBowl.addColorStop(0, "#f8fafc");
  outerBowl.addColorStop(0.7, "#cbd5e1");
  outerBowl.addColorStop(1, "#64748b");
  ellipseFill(cx, cy, 170, 170, outerBowl);

  const innerBowl = ctx.createRadialGradient(cx, cy, 20, cx, cy, 146);
  innerBowl.addColorStop(0, "#ffffff");
  innerBowl.addColorStop(0.75, "#e2e8f0");
  innerBowl.addColorStop(1, "#94a3b8");
  ellipseFill(cx, cy, 146, 146, innerBowl);

  ellipseFill(cx, cy, 62, 62, "#475569");
  ellipseFill(cx, cy, 52, 52, "#94a3b8");

  // Rotating bowl highlight spokes
  const spinAng = g.tick * 0.18;
  for (let i = 0; i < 4; i++) {
    const a = spinAng + i * (PI / 2);
    line(cx + Math.cos(a) * 65, cy + Math.sin(a) * 65, cx + Math.cos(a) * 135, cy + Math.sin(a) * 135, "rgba(255, 255, 255, 0.45)", 3);
  }

  // Sugar swirl trail from pointer
  if (g.spinTrail.length > 1) {
    ctx.save();
    for (let i = 1; i < g.spinTrail.length; i++) {
      const pt0 = g.spinTrail[i - 1];
      const pt1 = g.spinTrail[i];
      const alpha = i / g.spinTrail.length;
      ctx.strokeStyle = `rgba(255, 182, 217, ${alpha * 0.75})`;
      ctx.lineWidth = alpha * 12;
      ctx.lineCap = "round";
      ctx.beginPath();
      ctx.moveTo(pt0.x, pt0.y);
      ctx.lineTo(pt1.x, pt1.y);
      ctx.stroke();
    }
    ctx.restore();
  }

  // Floss & earned cotton puffs
  const sc = stallColours[g.players[g.currentPlayer].stallColour];
  drawCandy(cx, cy - 36, sc, 1.0);

  for (let i = 0; i < g.bonusEarned; i++) {
    const a = i * PI * 0.42 + (g.tick * 0.12);
    const px = cx + Math.cos(a) * (28 + i * 4.2);
    const py = cy - 16 + Math.sin(a) * (22 + i * 3.5);
    ellipseFill(px, py, 16, 16, i % 2 ? adjustColor(sc, 45) : sc);
    ellipseFill(px - 4, py - 4, 6, 6, "rgba(255, 255, 255, 0.5)");
  }

  // Floating +1 particles
  for (let i = g.spinParticles.length - 1; i >= 0; i--) {
    const pt = g.spinParticles[i];
    pt.y -= 1.6;
    pt.life -= 0.03;
    if (pt.life <= 0) {
      g.spinParticles.splice(i, 1);
    } else {
      ctx.save();
      ctx.globalAlpha = pt.life;
      centreText(pt.x - 20, pt.y - 12, pt.x + 20, pt.y + 12, "+1", "#fde047", "800 18px Outfit, sans-serif");
      ctx.restore();
    }
  }

  // Timer & Bonus Counter
  const timeStr = `00:0${Math.floor(remainingMs / 1000)}.${Math.floor((remainingMs % 1000) / 100)}`;
  centreText(cx - 100, cy + 194, cx + 100, cy + 232, timeStr, "#ffffff", "800 32px Outfit, sans-serif");

  const bonusStr = g.lang === LANG_EN ? `BONUS FLOSS CRAFTED: +${g.bonusEarned} STICKS` : `획득한 보너스 솜사탕: +${g.bonusEarned}개`;
  centreText(100, 534, 860, 574, bonusStr, "#34d399", "800 26px Outfit, sans-serif");

  if (remainingMs <= 0) {
    g.players[g.currentPlayer].bonusStock = g.bonusEarned;
    g.screen = SCREEN_PLAN;
    playSound(SOUND_WIN);
  }
}

// Screen 4: Plan Screen
function upfrontCost() {
  const p = g.players[g.currentPlayer];
  return p.stock * g.unitCostPence + p.adverts * 150;
}

function drawPlan() {
  const p = g.players[g.currentPlayer];
  ctx.fillStyle = "#f8fafc";
  ctx.fillRect(0, 0, APP_W, APP_H);
  drawScene();

  // Right Planning Sidebar Card
  ctx.save();
  ctx.shadowColor = "rgba(15, 23, 42, 0.16)";
  ctx.shadowBlur = 20;
  ctx.shadowOffsetX = -4;
  ctx.fillStyle = "#ffffff";
  ctx.fillRect(640, 0, 320, 600);
  ctx.restore();

  // Top Header Strip (positioned below top-right overlay controls)
  fillRound(656, 54, 288, 120, 16, "#f8fafc");
  strokeRound(656, 54, 288, 120, 16, "#e2e8f0", 1);

  textLeft(670, 64, g.lang === LANG_EN ? `DAY ${g.day} / ${g.totalDays}` : `${g.day}일차 / 총 ${g.totalDays}일`, "#0f172a", "800 16px Outfit, sans-serif");
  textLeft(834, 64, moneyStr(p.cashPence), "#059669", "800 16px Outfit, sans-serif");

  textLeft(670, 88, p.businessName, stallColours[p.stallColour], "700 15px Outfit, sans-serif");
  if (p.debtPence > 0) {
    const debtMsg = g.lang === LANG_EN ? `Debt: ${moneyStr(p.debtPence)}` : `대출금: ${moneyStr(p.debtPence)}`;
    textLeft(822, 89, debtMsg, "#e11d48", "700 13px Outfit, sans-serif");
  }

  const zName = g.lang === LANG_EN ? beachZoneNamesEN[g.currentPlayer] : beachZoneNamesKO[g.currentPlayer];
  const locName = g.lang === LANG_EN ? locationsData[g.loc].nameEN : locationsData[g.loc].nameKO;
  textLeft(670, 112, `${locName} • ${zName}`, "#64748b", "600 12.5px Outfit, sans-serif");

  const wName = g.lang === LANG_EN ? weatherNamesEN[g.weather] : weatherNamesKO[g.weather];
  textLeft(670, 132, g.lang === LANG_EN ? `${wName} • Crowd ${g.crowd}` : `${wName} • 유동인구 ${g.crowd}명`, "#1e293b", "700 14.5px Outfit, sans-serif");

  textLeft(670, 152, g.lang === LANG_EN ? `Cost ${g.unitCostPence}p  •  Suggested ${g.suggestedPricePence}p  •  Bonus +${p.bonusStock}` : `원가 ${g.unitCostPence}p • 권장 ${g.suggestedPricePence}p • 보너스 +${p.bonusStock}`, "#64748b", "600 12px Outfit, sans-serif");

  sliders[0].value = p.stock;
  sliders[1].value = p.pricePence;
  sliders[2].value = p.adverts;

  drawSlider(sliders[0], g.lang === LANG_EN ? "Stock to make" : "제조할 재고량", `${p.stock + p.bonusStock} (+${p.bonusStock})`);
  drawSlider(sliders[1], g.lang === LANG_EN ? "Price per candyfloss" : "개당 판매 가격", moneyStr(p.pricePence));
  drawSlider(sliders[2], g.lang === LANG_EN ? "Advertising signs" : "홍보 표지판 개수", `${p.adverts}`);

  planButtons[1].text = g.lang === LANG_EN ? "BORROW £50" : "£50 대출";
  planButtons[2].text = g.lang === LANG_EN ? "REPAY £50" : "£50 상환";
  drawButton(planButtons[1], "rgb(245, 158, 11)");
  const canRepay = p.debtPence > 0 && p.cashPence >= 5000;
  drawButton(planButtons[2], "rgb(37, 150, 235)", canRepay);

  const uCost = upfrontCost();
  const canAfford = uCost <= p.cashPence;
  fillRound(668, 466, 264, 34, 10, canAfford ? "#f0fdf4" : "#fff1f2");
  textLeft(682, 475, g.lang === LANG_EN ? "Up-front cost" : "사전 준비 비용", "#475569", "700 14px Outfit, sans-serif");
  textLeft(842, 475, moneyStr(uCost), canAfford ? "#059669" : "#e11d48", "800 15px Outfit, sans-serif");

  planButtons[0].text = g.lang === LANG_EN ? "OPEN FOR BUSINESS" : "영업 개시";
  drawButton(planButtons[0], "rgb(244, 63, 142)", canAfford);

  // Weather badge on upper left scene
  ctx.save();
  ctx.shadowColor = "rgba(15, 23, 42, 0.16)";
  ctx.shadowBlur = 10;
  ctx.shadowOffsetY = 3;
  fillRound(16, 16, 172, 40, 20, "rgba(255, 255, 255, 0.92)");
  ctx.restore();
  drawWeatherIconMini(38, 36, g.weather);
  textLeft(60, 27, wName, "#0f172a", "700 14px Outfit, sans-serif");
}

function runDay() {
  const p = g.players[g.currentPlayer];
  g.eventId = irand(0, 6);
  let eventDemand = 1.0;
  const totalStock = p.stock + p.bonusStock;
  let usable = totalStock;
  let effectiveCost = g.unitCostPence;
  let rep = 0;

  p.totalStockMade += totalStock;

  if (g.eventId === 1) eventDemand = 1.28;
  else if (g.eventId === 2) usable = Math.floor(usable * 0.82);
  else if (g.eventId === 3) eventDemand = 0.78;
  else if (g.eventId === 4) eventDemand = 1.35;
  else if (g.eventId === 5) effectiveCost = Math.floor(effectiveCost * 1.16);
  else if (g.eventId === 6) { eventDemand = 1.14; rep = 4; }

  const ratio = p.pricePence / g.suggestedPricePence;
  let priceEffect = 1.0;
  if (ratio <= 0.72) priceEffect = 1.28;
  else if (ratio <= 0.92) priceEffect = 1.13;
  else if (ratio <= 1.08) priceEffect = 1.0;
  else if (ratio <= 1.30) priceEffect = 0.78;
  else if (ratio <= 1.60) priceEffect = 0.48;
  else priceEffect = 0.18;

  const adEffect = 1.0 + Math.min(0.5, p.adverts * 0.035);
  const repEffect = 0.72 + p.reputation / 180.0;
  const random = irand(88, 112) / 100.0;
  const diff = g.difficulty === 0 ? 1.08 : (g.difficulty === 2 ? 0.93 : 1.0);

  p.customers = Math.floor(g.crowd * weatherDemand[g.weather] * priceEffect * adEffect * repEffect * random * eventDemand * diff + 0.5);
  if (p.customers < 0) p.customers = 0;

  p.sold = Math.min(p.customers, usable);
  p.unsold = totalStock - p.sold;
  p.revenuePence = p.sold * p.pricePence;
  p.costPence = p.stock * effectiveCost + p.adverts * 150;
  p.profitPence = p.revenuePence - p.costPence;
  p.cashPence += p.profitPence;

  if (p.debtPence > 0) {
    p.debtPence = Math.floor(p.debtPence * 1.05);
  }

  if (p.sold === 0) rep -= 4;
  else {
    if (ratio <= 1.05) rep += 1;
    if (ratio > 1.35) rep -= 3;
    if (p.sold >= usable && p.customers > usable) rep -= 1;
    if (p.unsold > totalStock * 0.45) rep -= 1;
    if (p.adverts >= 8) rep += 1;
  }
  p.reputation = clampi(p.reputation + rep, 5, 100);
  p.totalSold += p.sold;
  if (p.profitPence > p.bestProfit) p.bestProfit = p.profitPence;

  g.screen = SCREEN_RESULTS;
  playSound(p.profitPence >= 0 ? SOUND_PROFIT : SOUND_LOSS);
}

// Screen 5: Results
function drawResults() {
  const p = g.players[g.currentPlayer];
  drawModernBackdrop("#1e1b4b", "#4c1d95", "#6d28d9");
  drawCardPanel(100, 42, 760, 530, 30);

  if (g.numPlayers > 1) {
    const pName = g.lang === LANG_EN ? playerNamesEN[g.currentPlayer] : playerNamesKO[g.currentPlayer];
    const header = g.lang === LANG_EN ? `${pName} (${p.businessName}) - DAY ${g.day} RESULTS` : `${pName} (${p.businessName}) - ${g.day}일차 결과`;
    centreText(150, 54, 810, 88, header, stallColours[p.stallColour], "700 18px Outfit, sans-serif");
  }

  const eName = g.lang === LANG_EN ? eventNamesEN[g.eventId] : eventNamesKO[g.eventId];
  const eText = g.lang === LANG_EN ? eventTextsEN[g.eventId] : eventTextsKO[g.eventId];

  centreText(150, 84, 810, 126, eName, "#0f172a", "800 28px Outfit, sans-serif");
  centreText(170, 124, 790, 160, eText, "#64748b", "600 15px Outfit, sans-serif");

  const profStr = (p.profitPence > 0 ? "+" : "") + moneyStr(p.profitPence);
  fillRound(300, 166, 360, 68, 20, p.profitPence >= 0 ? "#ecfdf5" : "#fff1f2");
  centreText(300, 166, 660, 234, profStr, p.profitPence >= 0 ? "#059669" : "#e11d48", "900 44px Outfit, sans-serif");

  const labsEN = ["Customers", "Sold", "Unsold", "Revenue", "Costs", "Cash Total"];
  const labsKO = ["방문 손님", "판매 수량", "남은 재고", "총 매출", "총 비용", "보유 자산"];
  const vals = [p.customers, p.sold, p.unsold, p.revenuePence, p.costPence, p.cashPence];

  for (let i = 0; i < 6; i++) {
    const col = i % 3, row = Math.floor(i / 3);
    const left = 160 + col * 220, top = 254 + row * 102;
    fillRound(left, top, 196, 86, 18, "#f8fafc");
    strokeRound(left, top, 196, 86, 18, "#e2e8f0", 1);
    centreText(left, top + 8, left + 196, top + 34, g.lang === LANG_EN ? labsEN[i] : labsKO[i], "#64748b", "700 13.5px Outfit, sans-serif");
    const valStr = i >= 3 ? moneyStr(vals[i]) : `${vals[i]}`;
    centreText(left, top + 34, left + 196, top + 76, valStr, "#0f172a", "800 22px Outfit, sans-serif");
  }

  if (g.currentPlayer < g.numPlayers - 1) {
    const nextPName = g.lang === LANG_EN ? playerNamesEN[g.currentPlayer + 1] : playerNamesKO[g.currentPlayer + 1];
    resultButtons[0].text = g.lang === LANG_EN ? `NEXT PLAYER (${nextPName})` : `다음 플레이어 (${nextPName})`;
  } else if (g.day >= g.totalDays) {
    resultButtons[0].text = g.lang === LANG_EN ? "SEE FINAL LEADERBOARD" : "최종 순위표 보기";
  } else {
    resultButtons[0].text = g.lang === LANG_EN ? "NEXT DAY" : "다음 날로";
  }
  drawButton(resultButtons[0], "rgb(244, 63, 142)");
}

// Screen 6: Final Screen
function drawFinal() {
  drawModernBackdrop("#be185d", "#ec4899", "#fb7185");
  drawCardPanel(100, 44, 760, 526, 30);

  if (g.numPlayers === 1) {
    const p0 = g.players[0];
    const netWealth = p0.cashPence - p0.debtPence;
    const score = netWealth + p0.reputation * 45 + p0.totalSold * 5;

    let titleStr = "";
    if (netWealth < 0) titleStr = g.lang === LANG_EN ? "STALL CLOSED (BANKRUPT)" : "영업 종료 (파산)";
    else if (score >= 12000) titleStr = g.lang === LANG_EN ? "CANDYFLOSS CHAMPION!" : "솜사탕 챔피언!";
    else titleStr = g.lang === LANG_EN ? "SEASON COMPLETE" : "시즌 완수!";

    centreText(150, 62, 810, 118, titleStr, "#0f172a", "800 32px Outfit, sans-serif");

    const stats = [
      { l: g.lang === LANG_EN ? "NET WEALTH" : "순 자산", v: moneyStr(netWealth), c: "#059669" },
      { l: g.lang === LANG_EN ? "REPUTATION" : "평판", v: `${p0.reputation}`, c: "#0284c7" },
      { l: g.lang === LANG_EN ? "TOTAL SOLD" : "총 판매량", v: `${p0.totalSold}`, c: "#db2777" },
      { l: g.lang === LANG_EN ? "BEST PROFIT" : "최대 일일이익", v: moneyStr(p0.bestProfit), c: "#d97706" }
    ];
    for (let i = 0; i < 4; i++) {
      const col = i % 2, row = Math.floor(i / 2);
      const sx = 190 + col * 300, sy = 148 + row * 150;
      fillRound(sx, sy, 270, 120, 20, "#f8fafc");
      strokeRound(sx, sy, 270, 120, 20, "#e2e8f0", 1);
      centreText(sx, sy + 14, sx + 270, sy + 44, stats[i].l, "#64748b", "700 14px Outfit, sans-serif");
      centreText(sx, sy + 44, sx + 270, sy + 106, stats[i].v, stats[i].c, "800 32px Outfit, sans-serif");
    }

    if (!g.highScoreRecorded) {
      checkAndAddHighScore(playerNamesEN[0], p0.businessName, locationsData[g.loc].nameEN, score, netWealth, p0.reputation, p0.totalSold);
      g.highScoreRecorded = true;
    }
  } else {
    centreText(150, 56, 810, 106, g.lang === LANG_EN ? "FINAL LEADERBOARD" : "최종 순위표", "#0f172a", "800 32px Outfit, sans-serif");

    const order = [];
    const scores = [];
    for (let i = 0; i < g.numPlayers; i++) {
      order.push(i);
      const netW = g.players[i].cashPence - g.players[i].debtPence;
      scores[i] = netW + g.players[i].reputation * 45 + g.players[i].totalSold * 5;
    }
    order.sort((a, b) => scores[b] - scores[a]);

    const ranksEN = ["1st WINNER", "2nd PLACE", "3rd PLACE", "4th PLACE"];
    const ranksKO = ["1위 (우승)", "2위", "3위", "4위"];
    const rankCols = ["#d97706", "#64748b", "#b45309", "#64748b"];

    for (let i = 0; i < g.numPlayers; i++) {
      const pIdx = order[i];
      const pl = g.players[pIdx];
      const ry = 122 + i * 82;
      fillRound(140, ry, 680, 68, 18, i === 0 ? "#fefce8" : "#f8fafc");
      strokeRound(140, ry, 680, 68, 18, i === 0 ? "#fde047" : "#e2e8f0", 1.5);

      centreText(152, ry, 265, ry + 68, g.lang === LANG_EN ? ranksEN[i] : ranksKO[i], rankCols[i], "800 17px Outfit, sans-serif");
      const pName = g.lang === LANG_EN ? playerNamesEN[pIdx] : playerNamesKO[pIdx];
      textLeft(274, ry + 23, `${pName} (${pl.businessName})`, stallColours[pl.stallColour], "700 16px Outfit, sans-serif");

      const netW = pl.cashPence - pl.debtPence;
      centreText(470, ry, 580, ry + 68, moneyStr(netW), "#059669", "800 17px Outfit, sans-serif");
      centreText(585, ry, 690, ry + 68, g.lang === LANG_EN ? `Rep: ${pl.reputation}` : `평판: ${pl.reputation}`, "#0284c7", "700 16px Outfit, sans-serif");
      centreText(695, ry, 810, ry + 68, g.lang === LANG_EN ? `Sold: ${pl.totalSold}` : `판매: ${pl.totalSold}`, "#db2777", "700 16px Outfit, sans-serif");

      if (i === 0 && !g.highScoreRecorded) {
        checkAndAddHighScore(pName, pl.businessName, locationsData[g.loc].nameEN, scores[pIdx], netW, pl.reputation, pl.totalSold);
        g.highScoreRecorded = true;
      }
    }
  }

  finalButtons[0].text = g.lang === LANG_EN ? "PLAY AGAIN" : "다시 시작";
  finalButtons[1].text = g.lang === LANG_EN ? "PERFORMANCE REPORT" : "성과 분석 보고서";

  drawButton(finalButtons[0], "rgb(244, 63, 142)");
  drawButton(finalButtons[1], "rgb(37, 150, 235)");
}

// Screen 7: Hall of Fame
function drawHallOfFame() {
  drawModernBackdrop("#0f172a", "#1e293b", "#334155");
  drawCardPanel(100, 44, 760, 526, 30);

  centreText(150, 58, 810, 112, g.lang === LANG_EN ? "HALL OF FAME LEGENDS" : "명예의 전당 전설들", "#0f172a", "800 30px Outfit, sans-serif");

  const ranks = ["1st", "2nd", "3rd", "4th", "5th"];
  const rankCols = ["#d97706", "#64748b", "#b45309", "#64748b", "#64748b"];

  for (let i = 0; i < 5; i++) {
    const e = g.highScores[i];
    const ry = 124 + i * 72;
    fillRound(140, ry, 680, 62, 16, i === 0 ? "#fefce8" : "#f8fafc");
    strokeRound(140, ry, 680, 62, 16, i === 0 ? "#fde047" : "#e2e8f0", 1);
    centreText(152, ry, 218, ry + 62, ranks[i], rankCols[i], "800 18px Outfit, sans-serif");

    if (!e || e.score === 0) {
      centreText(230, ry, 800, ry + 62, g.lang === LANG_EN ? "— Empty Slot —" : "— 기록 없음 —", "#94a3b8", "700 17px Outfit, sans-serif");
      continue;
    }

    textLeft(228, ry + 12, `${e.pName} (${e.bName})`, "#0f172a", "700 15.5px Outfit, sans-serif");
    textLeft(228, ry + 36, e.locName, "#64748b", "600 13px Outfit, sans-serif");

    centreText(445, ry, 580, ry + 62, moneyStr(e.cashPence), "#059669", "800 17px Outfit, sans-serif");
    centreText(588, ry, 692, ry + 62, `Rep: ${e.reputation}`, "#0284c7", "700 16px Outfit, sans-serif");
    centreText(696, ry, 810, ry + 62, `Score: ${e.score}`, "#db2777", "800 16px Outfit, sans-serif");
  }

  hofButtons[0].text = g.lang === LANG_EN ? "BACK TO TITLE" : "메뉴로 돌아가기";
  drawButton(hofButtons[0], "rgb(244, 63, 142)");
}

// Screen 8: Performance Analysis
function drawPerformanceAnalysis() {
  drawModernBackdrop("#1e1b4b", "#312e81", "#4c1d95");
  drawCardPanel(80, 34, 800, 540, 30);

  centreText(120, 46, 840, 92, g.lang === LANG_EN ? "PLAYER PERFORMANCE ANALYSIS" : "플레이어 경영 성과 분석", "#0f172a", "800 28px Outfit, sans-serif");

  const gradeCols = {
    S: "#eab308", A: "#0ea5e9", B: "#10b981",
    C: "#f97316", D: "#e11d48"
  };

  for (let i = 0; i < g.numPlayers; i++) {
    const { grade, title, tip, eff } = analyzePlayerPerformance(i);
    const rowY = 98 + i * 102;
    fillRound(110, rowY, 740, 90, 18, "#f8fafc");
    strokeRound(110, rowY, 740, 90, 18, "#e2e8f0", 1);

    fillGradRound(124, rowY + 11, 68, 68, 14, adjustColor(gradeCols[grade], 20), gradeCols[grade]);
    centreText(124, rowY + 11, 192, rowY + 79, grade, "#ffffff", "900 40px Outfit, sans-serif");

    const pName = g.lang === LANG_EN ? playerNamesEN[i] : playerNamesKO[i];
    textLeft(208, rowY + 12, `${pName} - ${g.players[i].businessName}`, stallColours[g.players[i].stallColour], "700 17px Outfit, sans-serif");
    textLeft(208, rowY + 35, `Award: ${title}`, "#1e293b", "700 14px Outfit, sans-serif");

    const effStr = g.lang === LANG_EN ? `Sales Efficiency: ${eff}%` : `판매 효율성: ${eff}%`;
    textLeft(642, rowY + 14, effStr, "#059669", "700 15px Outfit, sans-serif");

    textLeft(208, rowY + 58, tip, "#64748b", "600 13px Outfit, sans-serif");
  }

  analysisButtons[0].text = g.lang === LANG_EN ? "BACK TO LEADERBOARD" : "순위표로 돌아가기";
  drawButton(analysisButtons[0], "rgb(244, 63, 142)");
}

// Input Handlers
function prepareDay() {
  const roll = irand(1, 100);
  g.weather = roll <= 27 ? WEATHER_SUNNY : roll <= 52 ? WEATHER_BREEZY : roll <= 72 ? WEATHER_CLOUDY : roll <= 87 ? WEATHER_RAIN : roll <= 95 ? WEATHER_HEAT : WEATHER_STORM;

  const locMult = locationsData[g.loc].crowdMult;
  g.crowd = Math.floor((irand(82, 145) + (g.day - 1) * 3) * locMult);
  g.unitCostPence = clampi(42 + (g.day - 1) * 2 + irand(-4, 6), 25, 85);
  g.suggestedPricePence = Math.floor((235 + (g.day - 1) * 5 + irand(-12, 12)) * locationsData[g.loc].priceSens);
  g.currentPlayer = 0;

  for (let i = 0; i < g.numPlayers; i++) {
    const p = g.players[i];
    p.bonusStock = 0;
    p.stock = clampi(Math.floor(p.cashPence / g.unitCostPence), 10, 90);
    p.pricePence = Math.floor((g.suggestedPricePence + 5) / 10) * 10;
    p.adverts = p.cashPence > 1800 ? 3 : 0;
  }
}

function startSpinMiniGame() {
  g.spinStartTick = Date.now();
  g.spinTotalAngle = 0;
  g.bonusEarned = 0;
  g.spinTrail = [];
  g.spinParticles = [];
  g.screen = SCREEN_SPIN;
}

function setSliderFromMouse(index, x) {
  const s = sliders[index];
  x = clampi(x, s.r.left, s.r.right);
  s.value = s.minv + Math.floor(((x - s.r.left) / (s.r.right - s.r.left)) * (s.maxv - s.minv));
  const p = g.players[g.currentPlayer];
  if (s.id === 1) {
    s.value = Math.floor(s.value / 5) * 5;
    p.stock = s.value;
  } else if (s.id === 2) {
    s.value = Math.floor(s.value / 10) * 10;
    p.pricePence = s.value;
  } else {
    p.adverts = s.value;
  }
}

function updateDomLanguage() {
  const inspBtn = document.getElementById("btn-inspiration");
  const howBtn = document.getElementById("btn-how-to-play");
  if (inspBtn) {
    inspBtn.innerHTML = g.lang === LANG_EN ? "<span>🕹️</span> 1982 Original" : "<span>🕹️</span> 1982 원작";
  }
  if (howBtn) {
    howBtn.innerHTML = g.lang === LANG_EN ? "<span>📖</span> How to Play" : "<span>📖</span> 게임 설명";
  }
}

function updateCursor(x, y) {
  let pointer = false;
  if (g.screen === SCREEN_TITLE) {
    pointer = titleButtons.some(b => ptIn(b.r, x, y)) || ptIn(titleInspirationBtn.r, x, y);
  } else if (g.screen === SCREEN_SETUP) {
    if (setupButtons.some(b => ptIn(b.r, x, y))) pointer = true;
    for (let i = 0; i < 7; i++) {
      if (ptIn({ left: 60 + i * 122, top: 102, right: 175 + i * 122, bottom: 146 }, x, y)) pointer = true;
    }
    for (let i = 0; i < 4; i++) {
      if (ptIn({ left: 60 + i * 140, top: 190, right: 185 + i * 140, bottom: 232 }, x, y)) pointer = true;
    }
    for (let i = 0; i < 3; i++) {
      if (ptIn({ left: 630 + i * 100, top: 190, right: 720 + i * 100, bottom: 232 }, x, y)) pointer = true;
      if (ptIn({ left: 630 + i * 100, top: 274, right: 720 + i * 100, bottom: 314 }, x, y)) pointer = true;
    }
    for (let i = 0; i < g.numPlayers; i++) {
      if (ptIn({ left: 60, top: 280 + i * 52, right: 500, bottom: 322 + i * 52 }, x, y)) pointer = true;
      if (ptIn({ left: 512, top: 280 + i * 52, right: 606, bottom: 322 + i * 52 }, x, y)) pointer = true;
    }
    if (g.numPlayers === 1) {
      for (let i = 0; i < 4; i++) {
        if (ptIn({ left: 630 + i * 74, top: 364, right: 694 + i * 74, bottom: 406 }, x, y)) pointer = true;
      }
    }
  } else if (g.screen === SCREEN_PASS_TURN) {
    pointer = ptIn(passButtons[0].r, x, y);
  } else if (g.screen === SCREEN_PLAN) {
    const p = g.players[g.currentPlayer];
    const canRepay = p.debtPence > 0 && p.cashPence >= 5000;
    const canOpen = upfrontCost() <= p.cashPence;
    if (sliders.some(s => ptIn(s.r, x, y))) pointer = true;
    if (ptIn(planButtons[1].r, x, y)) pointer = true;
    if (canRepay && ptIn(planButtons[2].r, x, y)) pointer = true;
    if (canOpen && ptIn(planButtons[0].r, x, y)) pointer = true;
  } else if (g.screen === SCREEN_RESULTS) {
    pointer = ptIn(resultButtons[0].r, x, y);
  } else if (g.screen === SCREEN_FINAL) {
    pointer = finalButtons.some(b => ptIn(b.r, x, y));
  } else if (g.screen === SCREEN_HOF) {
    pointer = ptIn(hofButtons[0].r, x, y);
  } else if (g.screen === SCREEN_ANALYSIS) {
    pointer = ptIn(analysisButtons[0].r, x, y);
  }
  canvas.style.cursor = pointer ? "pointer" : "default";
}

function clickAt(x, y) {
  initAudio();
  if (g.screen === SCREEN_TITLE) {
    if (ptIn(titleButtons[0].r, x, y)) {
      playSound(SOUND_CLICK);
      g.screen = SCREEN_SETUP;
    } else if (ptIn(titleButtons[1].r, x, y)) {
      playSound(SOUND_CLICK);
      showModal("how-to-play");
    } else if (ptIn(titleButtons[2].r, x, y)) {
      playSound(SOUND_CLICK);
      g.screen = SCREEN_HOF;
    } else if (ptIn(titleButtons[3].r, x, y)) {
      g.soundOn = !g.soundOn;
      playSound(SOUND_CLICK);
    } else if (ptIn(titleButtons[4].r, x, y)) {
      g.lang = g.lang === LANG_EN ? LANG_KO : LANG_EN;
      playSound(SOUND_CLICK);
      updateDomLanguage();
      for (let i = 0; i < 4; i++) {
        for (let k = 0; k < 6; k++) {
          if (g.players[i].businessName === defaultBizNamesEN[k] || g.players[i].businessName === defaultBizNamesKO[k]) {
            g.players[i].businessName = g.lang === LANG_EN ? defaultBizNamesEN[k] : defaultBizNamesKO[k];
            break;
          }
        }
      }
    } else if (ptIn(titleInspirationBtn.r, x, y)) {
      playSound(SOUND_CLICK);
      showModal("inspiration-modal");
    }
  } else if (g.screen === SCREEN_SETUP) {
    if (ptIn(setupButtons[1].r, x, y)) {
      playSound(SOUND_CLICK);
      g.screen = SCREEN_TITLE;
      return;
    }
    if (ptIn(setupButtons[0].r, x, y)) {
      playSound(SOUND_COIN);
      g.day = 1;
      g.highScoreRecorded = false;
      for (let i = 0; i < 4; i++) {
        g.players[i].active = i < g.numPlayers ? 1 : 0;
        g.players[i].cashPence = g.difficulty === 0 ? 4500 : g.difficulty === 2 ? 3000 : 3500;
        g.players[i].debtPence = 0;
        g.players[i].reputation = 50;
        g.players[i].totalSold = 0;
        g.players[i].totalStockMade = 0;
        g.players[i].bestProfit = -999999;
        if (g.numPlayers > 1) g.players[i].stallColour = i;
      }
      prepareDay();
      if (g.numPlayers > 1) {
        g.screen = SCREEN_PASS_TURN;
      } else {
        startSpinMiniGame();
      }
      return;
    }

    // Locations
    for (let i = 0; i < 7; i++) {
      if (ptIn({ left: 60 + i * 122, top: 102, right: 175 + i * 122, bottom: 146 }, x, y)) {
        playSound(SOUND_CLICK);
        g.loc = i;
      }
    }

    // Player Count
    for (let i = 0; i < 4; i++) {
      if (ptIn({ left: 60 + i * 140, top: 190, right: 185 + i * 140, bottom: 232 }, x, y)) {
        playSound(SOUND_CLICK);
        g.numPlayers = i + 1;
      }
    }

    // Days
    for (let i = 0; i < 3; i++) {
      if (ptIn({ left: 630 + i * 100, top: 190, right: 720 + i * 100, bottom: 232 }, x, y)) {
        playSound(SOUND_CLICK);
        g.totalDays = i === 0 ? 7 : i === 1 ? 10 : 14;
      }
    }

    // Difficulty
    for (let i = 0; i < 3; i++) {
      if (ptIn({ left: 630 + i * 100, top: 274, right: 720 + i * 100, bottom: 314 }, x, y)) {
        playSound(SOUND_CLICK);
        g.difficulty = i;
      }
    }

    // Business Name Box Click
    for (let i = 0; i < g.numPlayers; i++) {
      if (ptIn({ left: 60, top: 280 + i * 52, right: 500, bottom: 322 + i * 52 }, x, y)) {
        playSound(SOUND_CLICK);
        g.editingNamePlayer = i;
        const newName = prompt(g.lang === LANG_EN ? `Enter Business Name for Player ${i + 1}:` : `플레이어 ${i + 1}의 가게 이름을 입력하세요:`, g.players[i].businessName);
        if (newName && newName.trim().length > 0) {
          g.players[i].businessName = newName.trim().slice(0, 24);
        }
        g.editingNamePlayer = -1;
      }
      // Preset
      if (ptIn({ left: 512, top: 280 + i * 52, right: 606, bottom: 322 + i * 52 }, x, y)) {
        playSound(SOUND_CLICK);
        const rIdx = irand(0, 5);
        g.players[i].businessName = g.lang === LANG_EN ? defaultBizNamesEN[rIdx] : defaultBizNamesKO[rIdx];
      }
    }

    if (g.numPlayers === 1) {
      for (let i = 0; i < 4; i++) {
        if (ptIn({ left: 630 + i * 74, top: 364, right: 694 + i * 74, bottom: 406 }, x, y)) {
          playSound(SOUND_CLICK);
          g.players[0].stallColour = i;
        }
      }
    }
  } else if (g.screen === SCREEN_PASS_TURN) {
    if (ptIn(passButtons[0].r, x, y)) {
      playSound(SOUND_CLICK);
      startSpinMiniGame();
    }
  } else if (g.screen === SCREEN_PLAN) {
    for (let i = 0; i < 3; i++) {
      if (ptIn(sliders[i].r, x, y)) {
        g.activeSlider = i;
        setSliderFromMouse(i, x);
      }
    }

    // Borrow £50 Loan
    if (ptIn(planButtons[1].r, x, y)) {
      playSound(SOUND_COIN);
      g.players[g.currentPlayer].cashPence += 5000;
      g.players[g.currentPlayer].debtPence += 5250;
    }
    // Repay £50 Loan
    if (ptIn(planButtons[2].r, x, y) && g.players[g.currentPlayer].debtPence > 0 && g.players[g.currentPlayer].cashPence >= 5000) {
      playSound(SOUND_COIN);
      g.players[g.currentPlayer].cashPence -= 5000;
      g.players[g.currentPlayer].debtPence = Math.max(0, g.players[g.currentPlayer].debtPence - 5000);
    }

    if (ptIn(planButtons[0].r, x, y) && upfrontCost() <= g.players[g.currentPlayer].cashPence) {
      playSound(SOUND_COIN);
      runDay();
    }
  } else if (g.screen === SCREEN_RESULTS && ptIn(resultButtons[0].r, x, y)) {
    playSound(SOUND_CLICK);
    if (g.currentPlayer < g.numPlayers - 1) {
      g.currentPlayer++;
      if (g.numPlayers > 1) g.screen = SCREEN_PASS_TURN;
      else startSpinMiniGame();
    } else {
      if (g.day >= g.totalDays) {
        playSound(SOUND_WIN);
        g.screen = SCREEN_FINAL;
      } else {
        g.day++;
        prepareDay();
        if (g.numPlayers > 1) g.screen = SCREEN_PASS_TURN;
        else startSpinMiniGame();
      }
    }
  } else if (g.screen === SCREEN_FINAL) {
    if (ptIn(finalButtons[0].r, x, y)) {
      playSound(SOUND_CLICK);
      g.screen = SCREEN_SETUP;
    } else if (ptIn(finalButtons[1].r, x, y)) {
      playSound(SOUND_CLICK);
      g.screen = SCREEN_ANALYSIS;
    }
  } else if (g.screen === SCREEN_HOF && ptIn(hofButtons[0].r, x, y)) {
    playSound(SOUND_CLICK);
    g.screen = SCREEN_TITLE;
  } else if (g.screen === SCREEN_ANALYSIS && ptIn(analysisButtons[0].r, x, y)) {
    playSound(SOUND_CLICK);
    g.screen = SCREEN_FINAL;
  }
}

// Modal Dialog system
function showModal(id) {
  const modal = document.getElementById(id);
  if (modal) modal.classList.add("active");
}

function hideModal(id) {
  const modal = document.getElementById(id);
  if (modal) modal.classList.remove("active");
}

// Render Loop
function render() {
  g.tick++;
  syncCanvasResolution();
  ctx.clearRect(0, 0, APP_W, APP_H);

  switch (g.screen) {
    case SCREEN_TITLE: drawTitle(); break;
    case SCREEN_SETUP: drawSetup(); break;
    case SCREEN_PASS_TURN: drawPassTurn(); break;
    case SCREEN_SPIN: drawSpinningMiniGame(); break;
    case SCREEN_PLAN: drawPlan(); break;
    case SCREEN_RESULTS: drawResults(); break;
    case SCREEN_FINAL: drawFinal(); break;
    case SCREEN_HOF: drawHallOfFame(); break;
    case SCREEN_ANALYSIS: drawPerformanceAnalysis(); break;
  }

  requestAnimationFrame(render);
}

// Event Listeners Initialization
function initEvents() {
  canvas = document.getElementById("game-canvas");
  ctx = canvas.getContext("2d");

  loadHighScores();
  syncCanvasResolution();

  function getCanvasCoords(e) {
    const rect = canvas.getBoundingClientRect();
    const clientX = e.touches ? e.touches[0].clientX : e.clientX;
    const clientY = e.touches ? e.touches[0].clientY : e.clientY;
    const x = Math.floor(((clientX - rect.left) / rect.width) * APP_W);
    const y = Math.floor(((clientY - rect.top) / rect.height) * APP_H);
    return { x, y };
  }

  function handleSpinMove(x, y) {
    g.spinTrail.push({ x, y });
    if (g.spinTrail.length > 14) g.spinTrail.shift();

    const angle = Math.atan2(y - 292, x - 480);
    let diff = angle - g.lastMouseAngle;
    if (diff > PI) diff -= 2 * PI;
    if (diff < -PI) diff += 2 * PI;
    g.lastMouseAngle = angle;
    g.spinTotalAngle += Math.abs(diff);
    const newBonus = Math.min(25, Math.floor(g.spinTotalAngle / (PI * 1.8)));
    if (newBonus > g.bonusEarned) {
      g.bonusEarned = newBonus;
      g.spinParticles.push({ x, y, life: 1.0 });
      playSound(SOUND_CLICK);
    }
  }

  canvas.addEventListener("mousedown", (e) => {
    g.mouseDown = true;
    const { x, y } = getCanvasCoords(e);
    g.hoverX = x;
    g.hoverY = y;
    clickAt(x, y);
  });

  canvas.addEventListener("mousemove", (e) => {
    const { x, y } = getCanvasCoords(e);
    g.hoverX = x;
    g.hoverY = y;
    updateCursor(x, y);
    if (g.mouseDown && g.screen === SCREEN_PLAN && g.activeSlider >= 0) {
      setSliderFromMouse(g.activeSlider, x);
    }
    if (g.screen === SCREEN_SPIN) {
      handleSpinMove(x, y);
    }
  });

  canvas.addEventListener("mouseleave", () => {
    g.hoverX = -1;
    g.hoverY = -1;
    canvas.style.cursor = "default";
  });

  window.addEventListener("mouseup", () => {
    g.mouseDown = false;
    g.activeSlider = -1;
  });

  // Touch device support
  canvas.addEventListener("touchstart", (e) => {
    g.mouseDown = true;
    const { x, y } = getCanvasCoords(e);
    g.hoverX = x;
    g.hoverY = y;
    clickAt(x, y);
  }, { passive: true });

  canvas.addEventListener("touchmove", (e) => {
    const { x, y } = getCanvasCoords(e);
    g.hoverX = x;
    g.hoverY = y;
    if (g.mouseDown && g.screen === SCREEN_PLAN && g.activeSlider >= 0) {
      setSliderFromMouse(g.activeSlider, x);
    }
    if (g.screen === SCREEN_SPIN) {
      handleSpinMove(x, y);
    }
  }, { passive: true });

  canvas.addEventListener("touchend", () => {
    g.mouseDown = false;
    g.activeSlider = -1;
  });

  window.addEventListener("keydown", (e) => {
    if (e.key === "Escape") {
      const anyModalOpen = document.querySelector(".modal-overlay.active");
      if (anyModalOpen) {
        anyModalOpen.classList.remove("active");
        return;
      }
      if (g.screen !== SCREEN_TITLE) {
        g.screen = SCREEN_TITLE;
      }
    }
  });

  render();
}

window.addEventListener("DOMContentLoaded", initEvents);
