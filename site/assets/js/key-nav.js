/* key-nav.js
 * -----------------------------------------------------------------
 * Progressive enhancement: ← / → walk the catalog by reading the
 * <a rel="prev"> / <a rel="next"> links emitted by the scene-pager
 * (scenes/<slug>/) and the devlog post pager. Pure DOM + History;
 * no framework, no dependencies. Loaded with `defer` so it never
 * blocks rendering, and the page works fine without it.
 *
 * Bail-out conditions (so we don't hijack normal typing):
 *   - any modifier key (ctrl / meta / alt / shift) is held
 *   - focus is inside an editable element (input, textarea, select,
 *     [contenteditable])
 *   - the page has no rel=prev/next link (most pages)
 * -----------------------------------------------------------------
 */
(function () {
  "use strict";

  function isEditable(el) {
    if (!el) return false;
    var tag = el.tagName;
    if (tag === "INPUT" || tag === "TEXTAREA" || tag === "SELECT") return true;
    if (el.isContentEditable) return true;
    return false;
  }

  function findRel(rel) {
    var el = document.querySelector('a[rel~="' + rel + '"][href]');
    return el ? el.getAttribute("href") : null;
  }

  /* Reveal the "← / → arrow keys also walk this list" hint, but only
   * when this script is actually loaded — there's no point telling
   * a no-JS reader the keys work. The hint is hidden in HTML by
   * default and we drop the `hidden` attribute on init. */
  function revealHints() {
    if (!findRel("prev") && !findRel("next")) return;
    var hints = document.querySelectorAll(".scene-pager-hint[hidden]");
    for (var i = 0; i < hints.length; i++) {
      hints[i].removeAttribute("hidden");
    }
  }
  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", revealHints);
  } else {
    revealHints();
  }

  document.addEventListener("keydown", function (e) {
    if (e.ctrlKey || e.metaKey || e.altKey || e.shiftKey) return;
    if (isEditable(document.activeElement)) return;

    var target = null;
    if (e.key === "ArrowLeft")  target = findRel("prev");
    else if (e.key === "ArrowRight") target = findRel("next");
    if (!target) return;

    e.preventDefault();
    window.location.href = target;
  });
})();
