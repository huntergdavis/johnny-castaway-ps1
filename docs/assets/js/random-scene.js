/* random-scene.js
 * -----------------------------------------------------------------
 * Progressive enhancement: the "Random scene" link on /scenes/.
 * Reads scene-page URLs out of the rendered ledger table so the
 * data stays in sync with `_data/scenes.yml` without a duplicate
 * JS array. With JS off the trigger element is still a usable
 * link (it targets the scenes ledger anchor that contains it),
 * the rewrite below only happens when this script runs.
 *
 * The trigger is `<a data-random-scene-trigger>` placed once on
 * /scenes/. The script scopes its lookup to `table.scene-table`
 * so it won't fire on any other page that happens to load this
 * script. Loaded with `defer` so layout is settled before the
 * click handler attaches.
 * -----------------------------------------------------------------
 */
(function () {
  "use strict";

  function collectSceneLinks() {
    var table = document.querySelector("table.scene-table");
    if (!table) return [];
    var anchors = table.querySelectorAll("tbody td.scene-name a[href]");
    var hrefs = [];
    for (var i = 0; i < anchors.length; i++) {
      var href = anchors[i].getAttribute("href");
      if (href) hrefs.push(href);
    }
    return hrefs;
  }

  function pickRandom(arr) {
    if (!arr.length) return null;
    return arr[Math.floor(Math.random() * arr.length)];
  }

  function onReady() {
    var triggers = document.querySelectorAll("[data-random-scene-trigger]");
    if (!triggers.length) return;
    var hrefs = collectSceneLinks();
    if (!hrefs.length) return;
    for (var i = 0; i < triggers.length; i++) {
      var t = triggers[i];
      // Visually reveal the affordance when the enhancement is active.
      t.removeAttribute("hidden");
      t.addEventListener("click", function (e) {
        e.preventDefault();
        var pick = pickRandom(hrefs);
        if (pick) window.location.assign(pick);
      });
    }
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", onReady);
  } else {
    onReady();
  }
})();
