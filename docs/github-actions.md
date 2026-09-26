# GitHub Actions build and release

## Workflows

| Workflow | File | Purpose |
| --- | --- | --- |
| Windows release | `.github/workflows/windows-release.yml` | Builds Windows x64, publishes `AyuGram.zip` and updates the Scoop manifest. |

## Triggers

- Push of a `v*` tag: builds and publishes the GitHub release for that tag.
- Manual run from the Actions tab: always builds; publishes a release only when
  the optional `tag` input is filled in. This allows repackaging an already
  existing tag.
- The `debug_build` input switches the configuration from `Release` to `Debug`.

## Requirements

- `windows-2022` runner with Visual Studio 2022 (MSVC `v143`) and the Windows SDK.
- Python 3.10, configured by the workflow.
- Optional repository variables `TDESKTOP_API_ID` and `TDESKTOP_API_HASH`.
  When unset, the workflow falls back to the public Telegram Desktop test
  credentials `2040` / `b18441a1ff607e10a989891a5462e627`.
- Workflow permissions that allow writing repository contents (Settings ->
  Actions -> General -> Workflow permissions). Publishing a release and
  committing the Scoop manifest both need it; the workflow requests
  `contents: write` itself.

## Build steps

1. `Telegram/build/prepare/win.bat` builds the third-party libraries (Qt,
   OpenSSL, Breakpad, WebRTC and the rest) into the parent directory of the
   checkout: `../Libraries/win64` and `../ThirdParty`. This is the slow part of
   the job, so both directories are cached with a key derived from
   `prepare.py`, `qt_version.py` and the `cmake` submodule.
2. `Telegram/configure.bat x64 -DTDESKTOP_API_ID=... -DTDESKTOP_API_HASH=...`
   configures the Visual Studio solution in `out/`.
3. `cmake --build out --config Release --target Telegram` builds `AyuGram.exe`,
   `Updater.exe` and `modules/x64/d3d/d3dcompiler_47.dll`.
4. The three items above are packed into `AyuGram.zip` at the archive root,
   matching the layout expected by the Scoop manifest, and uploaded as the
   `AyuGram-windows-x64` artifact.

## Release and Scoop

When the build runs for a tag, a second job uploads `AyuGram.zip` to the
matching GitHub release (creating it when missing) and commits the new
`version`, `url` and `hash` to `bucket/ayugram.json` on `dev`.

## Caveats

- The `windows` job builds the third-party libraries from source on a cold
  cache, which can approach the six hour job limit and needs a lot of disk
  space. Warm the cache once with a manual run before relying on tag builds.
- GitHub keeps at most 10 GB of caches per repository. The library cache is
  large, so a save may be skipped if the limit is already reached; the build
  still succeeds, it just starts cold next time.
- If the `dev` branch is protected against direct pushes, the manifest update
  step fails. Run the update manually or allow `github-actions[bot]` to push to
  `dev`.

See [`scoop.md`](scoop.md) for the consumer-facing instructions.
