/* heading-anchors.js
 * -----------------------------------------------------------------
 * Progressive enhancement: hover-reveal "#" anchor links on H2-H6
 * inside <main>, so a reader can copy a deep-link to any subsection
 * without view-source. Skips h1 (the page-level title doesn't need
 * an in-page anchor) and any heading inside <details class="page-toc">
 * (the TOC itself emits a list of heading-links — appending one
 * inside its summary would double up).
 *
 * Pure DOM, no dependencies. Loaded via `defer` so it runs after
 * layout is settled — the anchors appear after first paint, which
 * is fine because they're hidden by default and only revealed on
 * hover/focus. The CSS for `.heading-anchor` lives in main.scss.
 * -----------------------------------------------------------------
 */
(function () {
  "use strict";

  function onReady() {
    var headings = document.querySelectorAll(
      "main h2[id], main h3[id], main h4[id], main h5[id], main h6[id]"
    );
    for (var i = 0; i < headings.length; i++) {
      var h = headings[i];
      // Skip TOC summaries and the TOC's own internal headings.
      if (h.closest && h.closest(".page-toc")) continue;
      // Skip if already augmented (defensive against double-runs).
      if (h.querySelector(".heading-anchor")) continue;
      var id = h.id;
      var label = (h.textContent || "").trim().replace(/\s+/g, " ");
      var a = document.createElement("a");
      a.className = "heading-anchor";
      a.href = "#" + id;
      a.setAttribute("aria-label", "Link to section: " + label);
      // Decorative glyph for sighted users; the aria-label above
      // carries the real semantics for screen-reader users.
      a.setAttribute("aria-hidden", "false");
      a.textContent = "#";
      h.appendChild(a);
    }
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", onReady);
  } else {
    onReady();
  }
})();
