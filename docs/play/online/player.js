const EMULATORJS_VERSION = "4.2.3";
const CORE_ASSET_VERSION = "20260714-230800";
const SETTINGS_SCHEMA_VERSION = 1;
const launchButton = document.querySelector("#launch-button");
const retryButton = document.querySelector("#retry-button");
const launchStage = document.querySelector("#launch-stage");
const gameSurface = document.querySelector("#game");
const launchStatus = document.querySelector("#launch-status");
const buildLabel = document.querySelector("#build-label");
const errorStage = document.querySelector("#error-stage");
const errorMessage = document.querySelector("#error-message");
const audioGate = document.querySelector("#audio-gate");
const audioState = document.querySelector("#audio-state");
const audioUnlockButton = document.querySelector("#audio-unlock-button");

let launchStarted = false;
let watchedAudioContext = null;

function installWebAudioVolumeBridge() {
  const emulator = window.EJS_emulator;
  const gain = emulator?.Module?.RWA?.gain;
  if (
    !emulator ||
    !gain ||
    typeof emulator.setVolume !== "function" ||
    emulator.jcVolumeBridgeInstalled
  ) {
    return;
  }

  const setVolume = emulator.setVolume.bind(emulator);
  emulator.setVolume = (volume) => {
    setVolume(volume);
    gain.gain.value = volume;
  };
  emulator.jcVolumeBridgeInstalled = true;
}

function formatMegabytes(bytes) {
  return `${(bytes / (1024 * 1024)).toFixed(1)} MB`;
}

function resolvePageUrl(value) {
  return new URL(value, window.location.href).href;
}

function showError(message) {
  launchStarted = false;
  launchButton.disabled = false;
  launchStage.hidden = true;
  audioGate.hidden = true;
  gameSurface.classList.remove("is-active");
  errorMessage.textContent = message;
  errorStage.hidden = false;
}

function emulatorAudioContext() {
  const module = window.EJS_emulator?.Module;
  return module?.RWA?.context ?? module?.AL?.currentCtx?.audioCtx ?? null;
}

function focusEmulator() {
  const target = window.EJS_emulator?.elements?.parent ?? gameSurface;
  if (typeof target?.focus === "function") {
    target.focus({ preventScroll: true });
  }
}

function selectConnectedGamepad() {
  const emulator = window.EJS_emulator;
  if (!emulator?.gamepadSelection || emulator.gamepadSelection[0]) {
    return;
  }

  const gamepad = emulator.gamepad?.gamepads?.find((candidate) => candidate);
  if (!gamepad) {
    return;
  }

  emulator.gamepadSelection[0] = `${gamepad.id}_${gamepad.index}`;
  if (typeof emulator.updateGamepadLabels === "function") {
    emulator.updateGamepadLabels();
  }
}

function forceAudibleVolume() {
  const emulator = window.EJS_emulator;
  if (!emulator) {
    return;
  }

  emulator.muted = false;
  emulator.volume = 1;
  installWebAudioVolumeBridge();
  if (typeof emulator.setVolume === "function") {
    try {
      emulator.setVolume(1);
    } catch (error) {
      console.warn("Could not apply browser audio volume", error);
    }
  }
}

function renderAudioState() {
  const context = emulatorAudioContext();
  if (!context) {
    audioState.textContent = "Audio unavailable";
    audioState.dataset.state = "failed";
    audioGate.hidden = true;
    return;
  }

  if (watchedAudioContext !== context) {
    watchedAudioContext = context;
    context.addEventListener("statechange", renderAudioState);
  }

  if (context.state === "running") {
    audioState.textContent = "Audio on";
    audioState.dataset.state = "running";
    audioGate.hidden = true;
    return;
  }

  audioState.textContent = "Audio locked";
  audioState.dataset.state = "pending";
  audioGate.hidden = false;
}

async function unlockAudio() {
  const context = emulatorAudioContext();
  if (!context) {
    renderAudioState();
    return;
  }

  forceAudibleVolume();
  try {
    await context.resume();
  } catch (error) {
    console.warn("Could not resume browser audio", error);
  }
  renderAudioState();
}

function unlockAudioFromGesture() {
  installWebAudioVolumeBridge();
  const context = emulatorAudioContext();
  const emulator = window.EJS_emulator;
  if (
    context &&
    (context.state !== "running" || emulator?.muted || emulator?.volume === 0)
  ) {
    void unlockAudio();
  }
}

function installEmulator(metadata) {
  window.EJS_player = "#game";
  window.EJS_core = "pcsx_rearmed";
  window.EJS_gameUrl = resolvePageUrl(metadata.cue_url);
  window.EJS_gameParentUrl = resolvePageUrl(metadata.bin_url);
  window.EJS_gameName = "Johnny Castaway";
  window.EJS_gameID = SETTINGS_SCHEMA_VERSION;
  window.EJS_biosUrl = "";
  window.EJS_pathtodata = "../../vendor/emulatorjs/data/";
  window.EJS_paths = {
    "pcsx_rearmed.json": resolvePageUrl(
      `../../vendor/emulatorjs/data/cores/reports/pcsx_rearmed.json?v=${CORE_ASSET_VERSION}`,
    ),
    "pcsx_rearmed-wasm.data": resolvePageUrl(
      `../../vendor/emulatorjs/data/cores/pcsx_rearmed-wasm.data?v=${CORE_ASSET_VERSION}`,
    ),
    "pcsx_rearmed-legacy-wasm.data": resolvePageUrl(
      `../../vendor/emulatorjs/data/cores/pcsx_rearmed-legacy-wasm.data?v=${CORE_ASSET_VERSION}`,
    ),
  };
  window.EJS_color = "#6ec8ff";
  window.EJS_volume = 1;
  window.EJS_threads = false;
  window.EJS_disableCue = true;
  window.EJS_CacheLimit = 0;
  window.EJS_disableDatabases = true;
  // A post-load gate supplies a fresh audio gesture after the context exists.
  window.EJS_startOnLoaded = true;
  window.EJS_startButtonName = "Run Johnny Castaway";
  window.EJS_language = "en-US";
  window.EJS_disableAutoLang = false;
  window.EJS_defaultOptions = {
    pcsx_rearmed_bios: "HLE",
    pcsx_rearmed_spu_interpolation: "gaussian",
  };
  window.EJS_defaultControls = {
    0: {
      0: { value: "k", value2: "BUTTON_1" },
      1: { value: "j", value2: "BUTTON_3" },
      2: { value: "backspace", value2: "SELECT" },
      3: { value: "enter", value2: "START" },
      4: { value: "up arrow", value2: "DPAD_UP" },
      5: { value: "down arrow", value2: "DPAD_DOWN" },
      6: { value: "left arrow", value2: "DPAD_LEFT" },
      7: { value: "right arrow", value2: "DPAD_RIGHT" },
      8: { value: "l", value2: "BUTTON_2" },
      9: { value: "i", value2: "BUTTON_4" },
      10: { value: "q", value2: "LEFT_TOP_SHOULDER" },
      11: { value: "e", value2: "RIGHT_TOP_SHOULDER" },
      12: { value: "1", value2: "LEFT_BOTTOM_SHOULDER" },
      13: { value: "3", value2: "RIGHT_BOTTOM_SHOULDER" },
      14: { value: "2", value2: "LEFT_STICK" },
      15: { value: "4", value2: "RIGHT_STICK" },
    },
    1: {},
    2: {},
    3: {},
  };
  window.EJS_ready = () => {
    buildLabel.textContent = `${metadata.build_label} / ${formatMegabytes(metadata.bin_size)}`;
  };
  window.EJS_onGameStart = () => {
    buildLabel.textContent = `${metadata.build_label} / running`;
    installWebAudioVolumeBridge();
    forceAudibleVolume();
    renderAudioState();
    selectConnectedGamepad();
    requestAnimationFrame(focusEmulator);
  };

  gameSurface.classList.add("is-active");
  launchStage.hidden = true;

  const loader = document.createElement("script");
  loader.src = `../../vendor/emulatorjs/data/loader.js?v=${EMULATORJS_VERSION}`;
  loader.onerror = () => showError("The pinned emulator runtime could not be loaded.");
  document.body.appendChild(loader);
}

async function launch() {
  if (launchStarted) {
    return;
  }

  launchStarted = true;
  launchButton.disabled = true;
  launchStatus.textContent = "Checking the latest playable build...";

  try {
    const response = await fetch("../../game/metadata.json", { cache: "no-store" });
    if (!response.ok) {
      throw new Error(`game server returned HTTP ${response.status}`);
    }

    const metadata = await response.json();
    if (!metadata.cue_url || !metadata.bin_url || !metadata.bin_size) {
      throw new Error("game server returned incomplete disc metadata");
    }

    launchStatus.textContent = `Loading ${formatMegabytes(metadata.bin_size)}...`;
    installEmulator(metadata);
  } catch (error) {
    showError(
      `${error.message}. Check the Johnny Castaway PS1 release page and try again.`,
    );
  }
}

launchButton.addEventListener("click", launch);
retryButton.addEventListener("click", () => {
  errorStage.hidden = true;
  launchStage.hidden = false;
  launchStatus.textContent = "Checking the latest playable build...";
  launch();
});

audioUnlockButton.addEventListener("click", async () => {
  await unlockAudio();
  focusEmulator();
});
document.addEventListener("pointerdown", unlockAudioFromGesture, true);
document.addEventListener("keydown", unlockAudioFromGesture, true);
document.addEventListener("touchstart", unlockAudioFromGesture, {
  capture: true,
  passive: true,
});
