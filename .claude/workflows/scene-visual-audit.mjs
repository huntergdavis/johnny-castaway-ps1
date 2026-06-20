export const meta = {
  name: 'scene-visual-audit',
  description: 'Capture + visually inspect scene variants (high+low tide) for capture-side defects: cut-off/edge-entry sprites, missing/blank frames, ghost sprites, thin-element/bubble dropout, sprite positioning',
  phases: [
    { title: 'Checklist', detail: 'red-team the visual-defect check list' },
    { title: 'Audit', detail: 'per-scene capture + montage + visual inspection' },
    { title: 'Synthesize', detail: 'collate prioritized release-candidate issue report' },
  ],
}

let ARGV = args
if (typeof ARGV === 'string') { try { ARGV = JSON.parse(ARGV) } catch (e) { /* leave as-is */ } }
const SCENES = ARGV && ARGV.scenes ? ARGV.scenes : ARGV
if (!Array.isArray(SCENES)) throw new Error('args must be {scenes:[...]} or [...]; got ' + typeof args)

const CHECKLIST_SCHEMA = {
  type: 'object', additionalProperties: false,
  required: ['checks'],
  properties: {
    checks: {
      type: 'array',
      items: {
        type: 'object', additionalProperties: false,
        required: ['id', 'name', 'howToSpot'],
        properties: {
          id: { type: 'string' },
          name: { type: 'string' },
          howToSpot: { type: 'string', description: 'how to detect it in host-capture frames/montages' },
        },
      },
    },
  },
}

const FINDINGS_SCHEMA = {
  type: 'object', additionalProperties: false,
  required: ['slug', 'captureOk', 'variants'],
  properties: {
    slug: { type: 'string' },
    captureOk: { type: 'boolean', description: 'true if frames were available/captured for inspection' },
    note: { type: 'string' },
    variants: {
      type: 'array',
      items: {
        type: 'object', additionalProperties: false,
        required: ['tide', 'clean', 'issues'],
        properties: {
          tide: { type: 'string', enum: ['high', 'low'] },
          clean: { type: 'boolean' },
          frameCount: { type: 'integer' },
          issues: {
            type: 'array',
            items: {
              type: 'object', additionalProperties: false,
              required: ['check', 'severity', 'description'],
              properties: {
                check: { type: 'string', description: 'checklist id' },
                severity: { type: 'string', enum: ['high', 'med', 'low'] },
                frames: { type: 'string' },
                description: { type: 'string' },
              },
            },
          },
        },
      },
    },
  },
}

phase('Checklist')
const seed = [
  'cut-off / edge-entry sprite: a sprite (boat, mermaid, shark, yacht) that should ENTER from a frame edge but instead pops in mid-frame, or is clipped at the left/right edge — check the EDGES montage (left/right strips). Known concern: boat/water-ski scenes (fishing4, activity9, fishing7, fishing8, visitor*) should have the craft enter from off-frame, not appear from nowhere.',
  'missing / blank frame: an all-black or all-magenta or duplicate-stuck frame mid-sequence, or a blank first/last frame — check ENDPOINTS + FULL montages.',
  'ghost sprite: a faint/dark duplicate of Johnny or a prop left over from a base-diff/keying artifact (the "ghost johnny" class).',
  'thin-element / bubble dropout: thought/speech-bubble connector dots or thin outlines present in the FULL frame but missing in the FGONLY frame (compare the DROPOUT montage). Bubble shells usually survive; thin dots/lines drop.',
  'sprite positioning / overlap: an actor or prop overlapping the island wrong, floating, or mis-placed vs the backdrop.',
  'wave/foam: island edge with no surf/foam (looks "fake/jagged").',
]
const checklistObj = await agent(
  `You are red-teaming a visual-QA checklist for a PS1 port of Johnny Castaway. We inspect HOST-CAPTURE frames (the reference render that feeds the foreground packs). Start from these known defect types and ADD any others worth checking for an "almost perfect" release candidate (small issues: clipped sprites, off-by-frames, palette/color glitches, halos, etc.). Keep each check detectable from sampled frame montages (full / left-right edges / endpoints / full-vs-fgonly). Known seeds:\n- ${seed.join('\n- ')}\nReturn the consolidated check list.`,
  { schema: CHECKLIST_SCHEMA, phase: 'Checklist' }
)
const checklistText = checklistObj.checks.map(c => `[${c.id}] ${c.name}: ${c.howToSpot}`).join('\n')
log(`Checklist: ${checklistObj.checks.length} checks`)

phase('Audit')
const results = await parallel(SCENES.map(s => () =>
  agent(
    `Visually audit scene "${s.slug}" (${s.name}) of the PS1 Johnny Castaway port for capture-side defects. Work in /home/hunter/workspace/jc_reborn.\n\n` +
    `STEP 0 — learn the scene's INTENDED content so you don't flag intended drama as a defect. Run:\n  cat site/scenes/${s.slug}/index.md 2>/dev/null | head -80\n  (and/or the matching caption in data/ps1/captions.json). This build replays the original Sierra animation faithfully, so big dramatic elements are usually CORRECT: e.g. an ENORMOUS boat that fills the whole screen (visitor3), a mermaid swimming up to the shoreline (mary3), a shark towing Johnny (fishing4), a yacht/boat passing (activity9). DO NOT report those as defects — they are the gag.\n\n` +
    `STEP 1 — ensure frames exist. First check disk:\n  ls host-results/${s.slug}-foreground-pilot/host-capture-high/frames/ 2>/dev/null | head -1\n  If frames are present, REUSE them (do NOT re-capture). If absent, capture now:\n  ./scripts/export-scene-foreground-pilot.sh "" ${s.slug} "${s.name}" ${s.base} 0 1.0 ${s.low}\n  (1-3 min; populates host-results/${s.slug}-foreground-pilot/. If it errors, set captureOk=false and explain in note.)\n\n` +
    `STEP 2 — build montages for BOTH tides:\n  ./scripts/audit-montage.sh ${s.slug} high\n  ./scripts/audit-montage.sh ${s.slug} low\n  Each writes /tmp/audit/${s.slug}/{high,low}_{full,edges,endpoints,dropout}.png and prints frames=N.\n\n` +
    `STEP 3 — Read (view) each montage that exists and inspect it against this checklist:\n${checklistText}\n\n` +
    `Report findings per tide. Be precise and HIGHLY SKEPTICAL — this build is "almost perfect", so a real defect is RARE and small. Only report something you can actually SEE in the montage (cite the frame label) AND that contradicts the intended content from STEP 0. A large sprite, a character at the shoreline, or a held final frame is NOT a defect if the scene description says so. Genuine defects look like: a sprite clipped mid-body at a frame edge while still moving, an unexpected blank/black/magenta frame mid-sequence, a faint duplicate ghost of a sprite, thin connector dots present in FULL but gone in fgonly. Empty issues + clean=true is the EXPECTED common case. Record frameCount from frames=N. Note any missing montage.`,
    { label: `audit:${s.slug}`, phase: 'Audit', schema: FINDINGS_SCHEMA }
  )
))

const ok = results.filter(Boolean)
phase('Synthesize')
const flat = ok.flatMap(r => (r.variants || []).map(v => ({ slug: r.slug, captureOk: r.captureOk, ...v })))
const withIssues = flat.filter(v => v.issues && v.issues.length > 0)
const report = await agent(
  `Synthesize a prioritized release-candidate visual-defect report for the PS1 Johnny Castaway port from these per-variant audit findings (JSON):\n\n${JSON.stringify({ scenesAudited: ok.length, variantsWithIssues: withIssues.length, findings: withIssues }, null, 2)}\n\n` +
  `Also list any scenes where captureOk=false (need re-capture). Group by defect type, order by severity (high first), and for each issue give scene+tide+frames+what to do (re-export with a lane carve / wider stitch / edge extension / etc.). Call out the boat/ski edge-entry scenes explicitly. Keep it tight and actionable.`,
  { phase: 'Synthesize' }
)
return { scenesAudited: ok.length, captureFailures: ok.filter(r => !r.captureOk).map(r => r.slug), variantsWithIssues: withIssues.length, report }
