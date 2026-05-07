/* table-sort.js
 * -----------------------------------------------------------------
 * Click-to-sort column headers for `<table class="scene-perf-table">`
 * (the perf battle card on /perf/). Pure DOM; no framework. Tries
 * numeric sort when the column's cell text looks numeric (handles
 * percentages, ± prefixes, and "1234/4567" ratio cells by sorting on
 * the leading number), falls back to lexical otherwise.
 *
 * Click a header → sort ascending. Click again → descending. Click a
 * different header → start ascending on the new key. The header gains
 * `aria-sort="ascending"` / `"descending"` so screen readers can
 * announce the order; a small ▲ / ▼ glyph is appended for sighted
 * readers.
 * -----------------------------------------------------------------
 */
(function () {
  "use strict";

  function parseNumeric(s) {
    if (s == null) return NaN;
    var trimmed = String(s).trim();
    if (trimmed === "" || trimmed === "—" || trimmed === "-") return NaN;
    // Take the first run that looks like a signed decimal number.
    var m = trimmed.match(/^([+-]?\d+(?:\.\d+)?)/);
    return m ? parseFloat(m[1]) : NaN;
  }

  function cellText(td) {
    return (td.textContent || "").trim();
  }

  function compareFactory(idx, dir) {
    return function (a, b) {
      var av = cellText(a.cells[idx]);
      var bv = cellText(b.cells[idx]);
      var na = parseNumeric(av);
      var nb = parseNumeric(bv);
      var bothNum = !isNaN(na) && !isNaN(nb);
      var cmp;
      if (bothNum) {
        cmp = na - nb;
      } else if (!isNaN(na)) {
        cmp = -1;          // numerics before non-numerics
      } else if (!isNaN(nb)) {
        cmp = 1;
      } else {
        cmp = av.localeCompare(bv, undefined, { numeric: true, sensitivity: "base" });
      }
      return dir === "asc" ? cmp : -cmp;
    };
  }

  function makeSortable(table) {
    var headerRow = table.tHead && table.tHead.rows[0];
    if (!headerRow) return;
    var tbody = table.tBodies[0];
    if (!tbody) return;

    Array.prototype.forEach.call(headerRow.cells, function (th, idx) {
      th.setAttribute("role", "button");
      th.setAttribute("tabindex", "0");
      th.setAttribute("aria-sort", "none");
      th.style.cursor = "pointer";
      th.style.userSelect = "none";

      function handleSort() {
        var current = th.getAttribute("aria-sort");
        var dir = current === "ascending" ? "descending" : "ascending";
        // Reset all other headers
        Array.prototype.forEach.call(headerRow.cells, function (other) {
          if (other !== th) {
            other.setAttribute("aria-sort", "none");
            var marker = other.querySelector(".sort-marker");
            if (marker) marker.textContent = "";
          }
        });
        th.setAttribute("aria-sort", dir);
        var marker = th.querySelector(".sort-marker");
        if (!marker) {
          marker = document.createElement("span");
          marker.className = "sort-marker";
          marker.style.marginLeft = "0.3em";
          th.appendChild(marker);
        }
        marker.textContent = dir === "ascending" ? "▲" : "▼";

        var rows = Array.prototype.slice.call(tbody.rows);
        rows.sort(compareFactory(idx, dir === "ascending" ? "asc" : "desc"));
        var frag = document.createDocumentFragment();
        rows.forEach(function (r) { frag.appendChild(r); });
        tbody.appendChild(frag);
      }

      th.addEventListener("click", handleSort);
      th.addEventListener("keydown", function (e) {
        if (e.key === "Enter" || e.key === " ") {
          e.preventDefault();
          handleSort();
        }
      });
    });
  }

  function init() {
    var tables = document.querySelectorAll("table.scene-perf-table");
    Array.prototype.forEach.call(tables, makeSortable);
  }
  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", init);
  } else {
    init();
  }
})();
