# CI/CD Strategy: Miele Matter Bridge PoC

**Status:** Proposed  
**Owner:** `build_tooling_steward`  
**Platform:** GitHub Actions

## 1. Goals
- **Verification:** Ensure the project compiles after every change.
- **Consistency:** Enforce code formatting and linting.
- **Transparency:** Provide build artifacts (binaries) for stakeholders without dev-env setup.
- **Independence:** Decouple firmware builds from local machine quirks.

## 2. Pipeline Structure

### A. Build Pipeline (on Push/PR to `main`)
1. **Checkout:** Get the source code.
2. **Environment Setup:** Use the official `espressif/idf:release-v5.1` Docker image.
3. **Submodule Sync:** Matter projects rely heavily on nested git submodules.
4. **Compile:** Run `idf.py build`.
5. **Artifact Upload:** Store `build/*.bin` and `sdkconfig`.

### B. Quality Gate (on PR)
1. **Format Check:** Verify `clang-format` compliance.
2. **Static Analysis:** Simple `cppcheck` (optional for PoC).

## 3. Secret Management
- **MIELE_CLIENT_ID / SECRET:** Stored as GitHub Actions Secrets.
- **TEST_CERTIFICATES:** Used only for CI builds, real certificates handled locally or via secret injection.

## 4. Release Strategy
- Every tag `v*` creates a **GitHub Release** with the compiled firmware bundle attached.

## 5. Non-Goals for PoC
- No automatic hardware flashing (HiL).
- No automatic deployment to Miele Cloud (API remains external).
- No production-grade security auditing in CI.
