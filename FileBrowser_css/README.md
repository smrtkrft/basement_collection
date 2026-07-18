# FileBrowser — Bakır

A warm dark theme for [FileBrowser](https://filebrowser.org). Charcoal ground,
copper accent, an engineering-paper grid backdrop — an instrument panel, not a
window. Type is IBM Plex Sans, with Plex Mono for sizes, dates, breadcrumbs
and labels.

It is **CSS only** — no code changes, no rebuild, no plugin. FileBrowser has a
built-in hook for a custom stylesheet, and that is all this uses.

Dark only: it looks the same whether or not FileBrowser's own dark mode is on.

The previous theme, **Midnight** (deep navy + steel blue), ships alongside as
`custom-midnight.css`. To switch back, swap the file names — FileBrowser only
reads `custom.css`.

## Install

**1. Get the folder onto your server**

```sh
git clone https://github.com/smrtkrft/basement_collection.git
```

Everything you need is in `basement_collection/FileBrowser_css`.

**2. Point FileBrowser at it**

```sh
filebrowser config set --branding.files /path/to/FileBrowser_css
```

Or in the UI: **Settings → Global Settings → Branding → Branding directory path**.

**3. Restart FileBrowser, then hard-reload your browser**

<kbd>Ctrl</kbd>/<kbd>Cmd</kbd> + <kbd>Shift</kbd> + <kbd>R</kbd>. The stylesheet
is cached hard — a normal reload usually shows the old look.

That's it.

### Docker

```yaml
services:
  filebrowser:
    image: filebrowser/filebrowser:latest
    volumes:
      - ./FileBrowser_css:/branding:ro
      - ./srv:/srv
      - ./database.db:/database.db
    environment:
      - FB_BRANDING_FILES=/branding
```

## Your logo

Replace `img/logo.png` with your own file — **keep the name**. It shows up in
the header and on the login screen.

Two things worth knowing:

- The folder **must** stay named `img/`. FileBrowser only serves `custom.css`
  and files under `img/` from the branding directory; anything else 404s.
- FileBrowser asks for `logo.svg`, so the theme redirects it to your PNG in
  CSS. If you'd rather use an SVG, name it `img/logo.svg` and delete the
  `content: url("img/logo.png")` rule in `custom.css` (section 13).

## Change the colour

One triplet re-tints icons, selection, focus and buttons. In `custom.css`:

```css
:root, :root.dark {
  --bk-accent: 210, 148, 100;   /* R, G, B — copper */
}
```

The backdrop is one warm hue — retune the mood with these two:

```css
  --bk-hue: 26;
  --bk-sat: 16%;
```

## Panel opacity

How solid the panels are — list rows, the header, the login card, settings
cards — is one number. `1` is fully opaque, lower is more glass:

```css
  --bk-veil: 0.78;   /* panels at 78% opacity */
```

Change it in `custom.css`, hard-reload, done.

## Empty folder

Bakır replaces the sad-face "lonely" message with a drop-zone panel: a cloud
icon, the original message text, and a helper line pointing at the upload
button. The helper line lives in **section 16** of `custom.css` — edit the
`content` string to change or remove it.

## Browser tab: name & icon

**The name** in the browser tab (and on the login screen and in the header) is
not CSS — it is FileBrowser's own branding setting:

```sh
filebrowser config set --branding.name "My Files"
```

Or in the UI: **Settings → Global Settings → Branding → Instance name**.

**The icon (favicon)**: FileBrowser serves anything under `img/` in the
branding folder *instead of* its built-in files. Create `img/icons/` and drop
your own icons in, keeping the exact names FileBrowser looks for:

```
img/icons/favicon.ico
img/icons/favicon-16x16.png
img/icons/favicon-32x32.png
img/icons/apple-touch-icon.png
```

Browsers cache favicons very aggressively — if the old icon sticks around,
test in a private window.

## Login background image

The image must live under `img/` — the only folder FileBrowser serves. Drop in
e.g. `img/login-bg.jpg`, then add this to **section 13** of `custom.css`:

```css
#login {
  background:
    linear-gradient(hsl(26 30% 5% / 0.5), hsl(26 30% 5% / 0.8)),
    url("img/login-bg.jpg") center / cover no-repeat fixed;
}
```

The gradient layered on top darkens the image so the login card stays
readable — tune the two alpha values to taste, or delete that line for the
raw image.

## Restyling the login screen

Everything on the login screen is styled in **section 13** of `custom.css`:
the card's width, position, blur, corners, inputs and button are all yours to
change. The one limit of a CSS-only theme: it cannot add new elements or text
to the page — existing ones can only be restyled, moved or hidden.

## Files

| File | What it is |
| --- | --- |
| `custom.css` | The active theme (**Bakır**). The only file FileBrowser reads. |
| `custom-midnight.css` | The previous theme (**Midnight**). Swap names to use it. |
| `img/logo.png` | Your logo. |
| `test.html` | Standalone design preview of Bakır — open in a browser. |
| `mockup-midnight-v2.html` | An earlier design concept. Not used by FileBrowser. |

FileBrowser ignores the extra files — it only ever serves `custom.css` and
`img/*`.

## Notes

- Fonts (IBM Plex) and Material Icons load from Google's CDN. Offline
  installs still work — the layout holds, only the typeface falls back.
- Written for FileBrowser v2. The theme targets stable selectors, but a major
  upstream frontend rewrite could require an update.

## Credits & thanks

**Huge thanks to the [FileBrowser](https://github.com/filebrowser/filebrowser)
team and its contributors** for building and freely sharing an excellent piece
of software. These themes are only a coat of paint — every bit of the actual
work, the file manager itself, is theirs. It also exists only because they
deliberately built a branding hook for exactly this purpose. Thank you.

Midnight's visual direction was inspired by
[topa-LE/filebrowser-custom-dark-theme](https://github.com/topa-LE/filebrowser-custom-dark-theme)
(GPL-2.0) — thanks to **topa-LE** for the idea and the look. The look is
borrowed; the code is independently written, so both files stay under the
repository's AGPL-3.0 licence. Bakır is an original design.

Typefaces: [IBM Plex](https://fonts.google.com/specimen/IBM+Plex+Sans) by IBM
(SIL Open Font License) and, in Midnight,
[Titillium Web](https://fonts.google.com/specimen/Titillium+Web) by Accademia
di Belle Arti di Urbino (SIL OFL). Icons:
[Material Icons](https://fonts.google.com/icons) by Google (Apache 2.0).
