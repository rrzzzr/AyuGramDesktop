# Windows releases and Scoop

This repository ships a self-hosted [Scoop](https://scoop.sh) bucket in
[`bucket/`](../bucket) and a GitHub Actions workflow that builds and publishes
the Windows release it points to.

## Install with Scoop

```powershell
scoop bucket add ayugram https://github.com/rrzzzr/AyuGramDesktop
scoop install ayugram/ayugram
```

`bucket/ayugram.json` installs the `AyuGram.exe`, `Updater.exe` and
`modules/x64/d3d/d3dcompiler_47.dll` payload from the `AyuGram.zip` asset of the
latest GitHub release, creates an "AyuGram" shortcut and persists the `tdata`
folder between updates.

## Update

```powershell
scoop update ayugram/ayugram
```

Scoop resolves new versions from the repository releases through the manifest
`checkver`/`autoupdate` fields, so no manual version bump is needed.

## How the manifest is maintained

The `Windows release` workflow (`.github/workflows/windows-release.yml`) runs on
every `v*` tag. Once the release and its `AyuGram.zip` asset exist, the workflow
computes the SHA256 of the archive and commits the updated `version`, `url` and
`hash` values to `bucket/ayugram.json` on the `dev` branch.

Before the first published release the manifest carries a `TODO` hash and is not
installable; it becomes valid as soon as the workflow has published one release.

## Using a different fork

If the repository is forked again, replace every `rrzzzr/AyuGramDesktop`
occurrence in `bucket/ayugram.json` with the new `owner/repo`. The workflow fills
in `version`, `url` and `hash` on the next tagged release.
