# Hollow Wilds (Unreal) — Native MCP Setup

How to wire **Claude Code** into the **native MCP server built into Unreal Engine 5.8**, so Claude can drive the Editor (spawn actors, build levels, create Blueprints/Materials/Niagara, run the GAS toolset, migrate assets, run automation tests, etc.).

> **This project is UE 5.8**, which ships Epic's official "Unreal MCP." You do **not** need any community MCP plugin (chongdashu/unreal-mcp, kvick-games/UnrealMCP, ChiR24/Unreal_mcp…) — those were pre-5.8 workarounds. Status: **experimental** (APIs may change), **loopback-only, no auth** (don't expose remotely), transport is **HTTP + SSE only** (no stdio/WebSocket).

---

## Architecture (two layers)

1. **Engine side** — an MCP *server* runs inside the Unreal Editor process. Tools are exposed via Unreal's `ToolsetRegistry`.
2. **Client side** — Claude Code connects to that server over HTTP at `http://127.0.0.1:8000/mcp`. You connect it either via an auto-generated `.mcp.json`, or via Epic's official **Claude Code plugin** (adds skills + hundreds of tools, recommended).

---

## A. Engine side — turn on the native server (do this in the Unreal Editor)

1. **Enable the plugins.** `Edit > Plugins`, then enable both:
   - **Unreal MCP** (its dependency *Toolset Registry* auto-enables).
   - **AllToolsets** — *required*. Unreal MCP exposes no tools on its own; AllToolsets registers them.
   Restart the Editor when prompted.

2. **(Optional) Auto-start the server.** `Edit > Editor Preferences > General > Model Context Protocol` → toggle **Auto Start Server**. Default bind is `http://127.0.0.1:8000/mcp` (port + path configurable here).

3. **Or start it manually** from the Editor console (the `~` console, or Output Log cmd line):
   ```
   ModelContextProtocol.StartServer
   ```
   (optionally `ModelContextProtocol.StartServer 8000` to pin a port).

4. **Generate the Claude Code client config** from the Editor console:
   ```
   ModelContextProtocol.GenerateClientConfig ClaudeCode
   ```
   This writes a `.mcp.json` to the **project root** (`...\Unreal Projects\HollowWilds\.mcp.json`) pointing at the running server. (Supported targets: `ClaudeCode`, `Cursor`, `VSCode`, `Gemini`, `Codex`, `All`.)

   ⚠️ **Merge caveat:** if you later add other MCP servers (Context7, ElevenLabs, Tripo, like your Unity project's `.mcp.json`), don't let `GenerateClientConfig` clobber them — keep the `unreal`/MCP server entry and merge the rest back in. Also add `.mcp.json` to `.gitignore` if it ends up holding any keys.

---

## B. Client side — connect Claude Code (pick ONE)

### Option 1 — Just use the generated `.mcp.json` (zero extra install)
After step A4, the `.mcp.json` in the project root already defines the Unreal HTTP server. Restart Claude Code from the project root, then run `/mcp` to confirm it shows **connected**. Done.

### Option 2 — Epic's official Claude Code plugin (recommended — richer)
Repo: `EpicGames/unreal-engine-skills-for-claude-code-plugin`. Adds Epic-authored **skills** + hundreds of auto-discovered tools across 30+ toolsets (actors, blueprints, materials, **Niagara**, Control Rigs, Sequencer, **State Trees**, widgets/UMG, **Gameplay Ability System**, automation testing, …).

1. Clone the repo locally (e.g. `C:\UnrealAssets\unreal-engine-skills-for-claude-code-plugin`).
2. In a Claude Code session at the project root:
   ```
   /plugin marketplace add C:\UnrealAssets\unreal-engine-skills-for-claude-code-plugin
   /plugin install unreal-engine-skills-for-claude-code@unreal-engine-skills-for-claude-code
   ```
   (Team alternative: reference the marketplace/plugin in `.claude/settings.json`.)
3. Ensure the Editor server is running (A1–A3) and the client config exists (A4).

---

## Prerequisites & gotchas

- **UE 5.8** ✓ (this project's EngineAssociation is 5.8).
- **Bash on PATH** — the Epic Claude Code plugin needs it. On Windows that means **Git Bash** (or WSL). You already have Git Bash.
- **Start order matters:** launch the Editor and start the MCP server *before* (re)starting Claude Code, or the client connects to nothing. Most failures are: plugin not enabled, **AllToolsets not enabled**, port 8000 conflict, or client started before the editor finished loading.
- **Restart Claude Code** after the config is generated / plugin installed — MCP servers load on a fresh start. Verify with `/mcp`.
- The server only works while the **Editor is open**.

---

## Why this matters for the migration

With the native MCP connected, Claude can do the **Editor-GUI work that's otherwise headless-blocked** in our plan (`Docs/Migration_Plan.md`): **Migrate the Synty Meadow** content + demo map, **import & retarget Hazel_V2**, author **Niagara** spell FX, and create the **Enhanced Input** assets + GAS ability/effect Blueprints that wire into the C++ scaffolding already in `Source/HollowWilds/`.

---

## Sources
- Epic official docs — Unreal MCP in Unreal Editor: https://dev.epicgames.com/documentation/unreal-engine/unreal-mcp-in-unreal-editor
- Epic Claude Code plugin: https://github.com/EpicGames/unreal-engine-skills-for-claude-code-plugin
