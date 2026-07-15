# FileBrowser — Midnight

A dark theme for [FileBrowser](https://filebrowser.org). Deep navy falling to
black, with a steel-blue accent.

It is **CSS only** — no code changes, no rebuild, no plugin. FileBrowser has a
built-in hook for a custom stylesheet, and that is all this uses.

Dark only: it looks the same whether or not FileBrowser's own dark mode is on.

## Install

**1. Get the folder onto your server**

```sh
git clone https://github.com/SmartKraft/basement_collection.git
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

Replace `img/logo.png` with your own file — **keep the name**. It shows up in the
header and on the login screen.

Two things worth knowing:

- The folder **must** stay named `img/`. FileBrowser only serves `custom.css` and
  files under `img/` from the branding directory; anything else 404s.
- FileBrowser asks for `logo.svg`, so the theme redirects it to your PNG in CSS.
  If you'd rather use an SVG, name it `img/logo.svg` and delete the
  `content: url("img/logo.png")` rule in `custom.css` (section 13).

## Change the colour

One value re-tints folders, selection, focus and buttons. In `custom.css`:

```css
:root, :root.dark {
  --tp-accent: 138, 170, 199;   /* R, G, B */
}
```

The background is one hue at four brightness steps — change these six and
nothing else:

```css
  --tp-hue: 215;
  --tp-sat: 40%;
  --tp-l-core: 24%;   /* lit centre        */
  --tp-l-mid:  15%;   /* falling off       */
  --tp-l-far:   8%;   /* approaching edges */
  --tp-l-edge:  4%;   /* corners           */
```

## Files

| File | What it is |
| --- | --- |
| `custom.css` | The theme. The only file FileBrowser reads. |
| `img/logo.png` | Your logo. |

## Notes

- Fonts and icons load from Google's CDN. Offline installs still work — the
  layout holds, only the typeface falls back.
- Written for FileBrowser v2. The theme targets stable selectors, but a major
  upstream frontend rewrite could require an update.

## Credits & thanks

**Huge thanks to the [FileBrowser](https://github.com/filebrowser/filebrowser)
team and its contributors** for building and freely sharing an excellent piece of
software. This theme is only a coat of paint — every bit of the actual work, the
file manager itself, is theirs. It also exists only because they deliberately
built a branding hook for exactly this purpose. Thank you.

Visual direction inspired by
[topa-LE/filebrowser-custom-dark-theme](https://github.com/topa-LE/filebrowser-custom-dark-theme)
(GPL-2.0) — thanks to **topa-LE** for the idea and the look. The look is
borrowed; the code is independently written, so this file stays under the
repository's AGPL-3.0 licence.

Typeface: [Titillium Web](https://fonts.google.com/specimen/Titillium+Web) by
Accademia di Belle Arti di Urbino (SIL Open Font License). Icons:
[Material Icons](https://fonts.google.com/icons) by Google (Apache 2.0).
