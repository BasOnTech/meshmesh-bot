# Versions, builds and publishing

`versions.properties` is the single source of the public version: `firmware` for all boards, a
`MAJOR.MINOR.PATCH` value: incompatible behaviour,
compatible features, then fixes. This does not change
the MeshCore wire protocol version.

Before a release, bump the affected component and run:

```sh
python tools/build/version.py --generate
python tools/build/version.py --check
```

The generated `src/core/Version.h` is committed and checked by PlatformIO. Firmware `status`
reports `version`, Git `revision` (with `-dirty` for local changes) and the image's `build_sha256`.
A matching version never replaces a hash check. Package `manifest.json` includes the version and
revision; site generation rejects packages from different revisions.

## On every push

A push to `main` or manual `.github/workflows/pages.yml` run:

1. Checks versions, translations of the screens and the page (`tools/gen/web_i18n.cjs check`).
2. Builds and packages all 18 targets, all nRF52 language images, ESP32 installer language markers
   and community-board factory images.
3. Generates site screenshots in every language; failure now blocks publication.
4. Assembles the site, chess page, ZIP packages, installer manifests, `release.json`. Checks hashes,
   the complete board list and consistent revisions.
5. Deploys the complete artifact to GitHub Pages only after every required job succeeds and while
   the commit is still the head of `main`. Failure leaves the previously published site available.

The site's `release.json` ties versions to the source revision and board image hashes. After
publishing, compare it with the pushed commit. Passing CI is
not hardware verification; record actual board results separately in `docs/verification.md`.

A push does not flash boards. Update through the site or USB. Firmware OTA is not implemented.
Documentation and `AGENTS.md` are edited before the commit. CI validates and publishes prepared
content; it does not invent hardware reports or update project instructions automatically.
