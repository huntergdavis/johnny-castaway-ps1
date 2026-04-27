---
layout: page
title: Holiday emblem gallery
eyebrow: Reference
subtitle: 32 pixel-art icons, 32x32 each, in the Sierra house style.
description: The 32-cell emblem sheet that overlays the island during a holiday window. Style guide, palette, and a per-icon table with sheet positions.
---

The four original Sierra holidays (New Year's Day, St. Patrick's
Day, Halloween, Christmas) keep their original full-island sprites.
The 31 holidays added by this port use a different artifact: a
32x32 transparent emblem that overlays the corner of the screen during
the holiday's window. There are 32 emblems total — the 31 added
holidays plus Elvis's Birthday, which received a fresh emblem because
the original game did not include it.

The icons were authored by AI sub-agents working from
[`docs/ps1/holidays-style-guide.md`]({{ site.github_url }}/blob/main/docs/ps1/holidays-style-guide.md)
and a shared 16-color CLUT defined in `scripts/holidays_art_lib.py`.
Index 0 is reserved for transparent. The visual register matches the
Sierra screensaver house style: saturated VGA primaries (cyan, blue,
yellow, red, green), 1px dark outlines where they help readability,
roughly 20-30 active pixels across, no Johnny, no palm reskins, no
sand or sky strips. Each emblem is a small overlay prop, not a scene.

All 32 cells fit on one 256x128 sheet, eight columns by four rows.
At runtime the PS1 reads a single cell out of the sheet by `(x, y)`
offset and blits it transparent over the screen at the
`(island_x, island_y)` anchor recorded in the holiday's row of
`gHolidays[]`.

## The sheet

<img src="{{ site.github_url }}/raw/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png" alt="Sheet of 32 holiday emblem sprites" style="image-rendering: pixelated; width: 100%; max-width: 512px; border: 3px solid var(--jc-trunk);" />

Direct link:
[`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png).
A checkerboard preview at the same path makes transparency obvious.

## Per-emblem index

Cell numbers run row-major from the top-left. `(x, y)` is the
pixel offset into the sheet. The slug column links to the holiday's
own page.

<table>
<thead><tr><th>ID</th><th>Holiday</th><th>Short name</th><th>Cell</th><th>(x, y)</th><th>Description</th></tr></thead>
<tbody>
<tr><td>5</td><td><a href="{{ '/docs/holidays/calendar/elvis-bday/' | relative_url }}">Elvis's Birthday</a></td><td><code>ELVIS BDAY</code></td><td>0</td><td>(0, 0)</td><td>Small guitar and music note.</td></tr>
<tr><td>6</td><td><a href="{{ '/docs/holidays/calendar/mlk-day/' | relative_url }}">MLK Jr. Day</a></td><td><code>MLK DAY</code></td><td>1</td><td>(32, 0)</td><td>Tiny podium with speech paper and dove.</td></tr>
<tr><td>7</td><td><a href="{{ '/docs/holidays/calendar/groundhog/' | relative_url }}">Groundhog Day</a></td><td><code>GROUNDHOG</code></td><td>2</td><td>(64, 0)</td><td>Groundhog head popping from a burrow.</td></tr>
<tr><td>8</td><td><a href="{{ '/docs/holidays/calendar/valentine/' | relative_url }}">Valentine's Day</a></td><td><code>VALENTINE</code></td><td>3</td><td>(96, 0)</td><td>Heart carved on trunk sliver with small floating hearts.</td></tr>
<tr><td>9</td><td><a href="{{ '/docs/holidays/calendar/super-bowl/' | relative_url }}">Super Bowl Sunday</a></td><td><code>SUPER BOWL</code></td><td>4</td><td>(128, 0)</td><td>Football with laces.</td></tr>
<tr><td>10</td><td><a href="{{ '/docs/holidays/calendar/presidents/' | relative_url }}">Presidents' Day</a></td><td><code>PRESIDENTS</code></td><td>5</td><td>(160, 0)</td><td>Tricorn hat with star cockade.</td></tr>
<tr><td>11</td><td><a href="{{ '/docs/holidays/calendar/mardi-gras/' | relative_url }}">Mardi Gras</a></td><td><code>MARDI GRAS</code></td><td>6</td><td>(192, 0)</td><td>Mardi Gras mask and beads.</td></tr>
<tr><td>12</td><td><a href="{{ '/docs/holidays/calendar/pi-day/' | relative_url }}">Pi Day</a></td><td><code>PI DAY</code></td><td>7</td><td>(224, 0)</td><td>Tiny chalkboard with pi and pie slice.</td></tr>
<tr><td>13</td><td><a href="{{ '/docs/holidays/calendar/spring/' | relative_url }}">First Day of Spring</a></td><td><code>SPRING</code></td><td>8</td><td>(0, 32)</td><td>Blossom branch with butterfly.</td></tr>
<tr><td>14</td><td><a href="{{ '/docs/holidays/calendar/april-fool/' | relative_url }}">April Fool's Day</a></td><td><code>APRIL FOOL</code></td><td>9</td><td>(32, 32)</td><td>Whoopee cushion and clown nose.</td></tr>
<tr><td>36</td><td><a href="{{ '/docs/holidays/calendar/420-day/' | relative_url }}">4/20 Day</a></td><td><code>420 DAY</code></td><td>10</td><td>(64, 32)</td><td>Green leaf and peace-sign medallion.</td></tr>
<tr><td>15</td><td><a href="{{ '/docs/holidays/calendar/easter/' | relative_url }}">Easter</a></td><td><code>EASTER</code></td><td>11</td><td>(96, 32)</td><td>Decorated Easter eggs.</td></tr>
<tr><td>16</td><td><a href="{{ '/docs/holidays/calendar/earth-day/' | relative_url }}">Earth Day</a></td><td><code>EARTH DAY</code></td><td>12</td><td>(128, 32)</td><td>Globe with tiny leaf.</td></tr>
<tr><td>17</td><td><a href="{{ '/docs/holidays/calendar/star-wars/' | relative_url }}">Star Wars Day</a></td><td><code>STAR WARS</code></td><td>13</td><td>(160, 32)</td><td>Lightsaber prop with star sparkle.</td></tr>
<tr><td>18</td><td><a href="{{ '/docs/holidays/calendar/cinco-mayo/' | relative_url }}">Cinco de Mayo</a></td><td><code>CINCO MAYO</code></td><td>14</td><td>(192, 32)</td><td>Sombrero and maraca.</td></tr>
<tr><td>19</td><td><a href="{{ '/docs/holidays/calendar/mothers-day/' | relative_url }}">Mother's Day</a></td><td><code>MOTHERS DAY</code></td><td>15</td><td>(224, 32)</td><td>Coconut vase bouquet.</td></tr>
<tr><td>20</td><td><a href="{{ '/docs/holidays/calendar/memorial/' | relative_url }}">Memorial Day</a></td><td><code>MEMORIAL</code></td><td>16</td><td>(0, 64)</td><td>Half-mast American flag with poppies.</td></tr>
<tr><td>21</td><td><a href="{{ '/docs/holidays/calendar/fathers-day/' | relative_url }}">Father's Day</a></td><td><code>FATHERS DAY</code></td><td>17</td><td>(32, 64)</td><td>Dad tie and small spatula.</td></tr>
<tr><td>22</td><td><a href="{{ '/docs/holidays/calendar/summer/' | relative_url }}">First Day of Summer</a></td><td><code>SUMMER</code></td><td>18</td><td>(64, 64)</td><td>Smiling summer sun with shades.</td></tr>
<tr><td>23</td><td><a href="{{ '/docs/holidays/calendar/pride/' | relative_url }}">Pride Day</a></td><td><code>PRIDE</code></td><td>19</td><td>(96, 64)</td><td>Rainbow pride flag with heart.</td></tr>
<tr><td>24</td><td><a href="{{ '/docs/holidays/calendar/july-4th/' | relative_url }}">Independence Day</a></td><td><code>JULY 4TH</code></td><td>20</td><td>(128, 64)</td><td>Firework burst and small flag.</td></tr>
<tr><td>25</td><td><a href="{{ '/docs/holidays/calendar/moon-land/' | relative_url }}">Moon Landing Day</a></td><td><code>MOON LAND</code></td><td>21</td><td>(160, 64)</td><td>Toy rocket and moon.</td></tr>
<tr><td>26</td><td><a href="{{ '/docs/holidays/calendar/watermelon/' | relative_url }}">National Watermelon Day</a></td><td><code>WATERMELON</code></td><td>22</td><td>(192, 64)</td><td>Watermelon slice with seeds.</td></tr>
<tr><td>27</td><td><a href="{{ '/docs/holidays/calendar/left-hand/' | relative_url }}">Left-Handers Day</a></td><td><code>LEFT HAND</code></td><td>23</td><td>(224, 64)</td><td>Left hand holding pencil.</td></tr>
<tr><td>28</td><td><a href="{{ '/docs/holidays/calendar/hawaii-day/' | relative_url }}">Hawaii Statehood Day</a></td><td><code>HAWAII DAY</code></td><td>24</td><td>(0, 96)</td><td>Hibiscus flower.</td></tr>
<tr><td>29</td><td><a href="{{ '/docs/holidays/calendar/labor-day/' | relative_url }}">Labor Day</a></td><td><code>LABOR DAY</code></td><td>25</td><td>(32, 96)</td><td>Hard hat and tool.</td></tr>
<tr><td>30</td><td><a href="{{ '/docs/holidays/calendar/pirate-day/' | relative_url }}">Talk Like a Pirate Day</a></td><td><code>PIRATE DAY</code></td><td>26</td><td>(64, 96)</td><td>Pirate tricorn and skull face.</td></tr>
<tr><td>31</td><td><a href="{{ '/docs/holidays/calendar/autumn/' | relative_url }}">First Day of Autumn</a></td><td><code>AUTUMN</code></td><td>27</td><td>(96, 96)</td><td>Autumn leaf and acorn.</td></tr>
<tr><td>32</td><td><a href="{{ '/docs/holidays/calendar/columbus/' | relative_url }}">Columbus / Indigenous Peoples' Day</a></td><td><code>COLUMBUS</code></td><td>28</td><td>(128, 96)</td><td>Compass rose on parchment map.</td></tr>
<tr><td>33</td><td><a href="{{ '/docs/holidays/calendar/election/' | relative_url }}">Election Day</a></td><td><code>ELECTION</code></td><td>29</td><td>(160, 96)</td><td>Ballot box with checked ballot.</td></tr>
<tr><td>34</td><td><a href="{{ '/docs/holidays/calendar/veterans/' | relative_url }}">Veterans Day</a></td><td><code>VETERANS</code></td><td>30</td><td>(192, 96)</td><td>Medal and wreath.</td></tr>
<tr><td>35</td><td><a href="{{ '/docs/holidays/calendar/thanksgive/' | relative_url }}">Thanksgiving</a></td><td><code>THANKSGIVE</code></td><td>31</td><td>(224, 96)</td><td>Cornucopia with fruit.</td></tr>
</tbody>
</table>

## Style rules

From [`docs/ps1/holidays-style-guide.md`]({{ site.github_url }}/blob/main/docs/ps1/holidays-style-guide.md):

- 32x32 canvas; transparent (palette index 0) background.
- 16-color CLUT shared across all emblems. No per-emblem palette.
- 20-30 active pixels across the icon. Readable from the PS1 viewing
  distance on a CRT.
- 1px dark outlines around the silhouette where contrast against the
  island helps. Skip outlines on bright accents that read fine on
  their own.
- No Johnny. No palm reskins. No sand or sky strips. The sprite is a
  small prop, not a scene reskin.
- The three-color hint in each row of `holidays.yml` (e.g. red /
  green / gold for Christmas) is a *guide* to the codegen step, not a
  full palette. The full palette is the shared CLUT.

## Adding an emblem

See the build pipeline reference:
[`docs/ps1/holidays-pipeline.md`]({{ site.github_url }}/blob/main/docs/ps1/holidays-pipeline.md).
Short version:

```bash
./scripts/holidays-build-all.sh --clean
```

That regenerates the per-icon PNGs, the packed sheet, the
checkerboard preview, and `manifest.json` into `scratch/holidays-emblems/`.
Tracked review copies live at
[`docs/ps1/holidays-emblems/`]({{ site.github_url }}/tree/main/docs/ps1/holidays-emblems).

## Related

- [Per-holiday calendar]({{ '/docs/holidays/' | relative_url }})
- [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }})
- [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) — source of truth
- [`scripts/holidays_art_lib.py`]({{ site.github_url }}/blob/main/scripts/holidays_art_lib.py) — shared CLUT
- [`docs/ps1/holidays-style-guide.md`]({{ site.github_url }}/blob/main/docs/ps1/holidays-style-guide.md) — full style rules
