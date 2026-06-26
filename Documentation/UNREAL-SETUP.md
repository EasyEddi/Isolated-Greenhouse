# Unreal Setup

## Version

Use Unreal Engine 5.8 for this project.

Both team members should use the same engine version. Do not mix 5.6, 5.7, 5.8, and preview versions.

## Git LFS

Git LFS is needed because Unreal assets like `.uasset` and `.umap` are binary files. Normal Git can store them, but the repository becomes slow and hard to work with. LFS keeps those files managed separately.

Install Git LFS before adding real Unreal assets:

```bash
git lfs install
```

The repository already contains `.gitattributes` rules for common Unreal asset files.

## First Unreal Steps

1. Install Unreal Engine 5.8.
2. Open `game/UnrealProject/TheIsolatedGreenhouse.uproject`.
3. Let Unreal generate any missing project files if it asks.
4. Create a first empty level named `L_Greenhouse_MVP` inside `Content/Maps`.
5. Save the level.
6. Commit the new Unreal files on branch `eddi/unreal-project-setup`.
7. Open a pull request and let Tarek review it.

## Agent Rule

When using an agent inside Unreal, keep tasks small. Ask for one system at a time:

- project setup
- first map
- interaction system
- Monstera plant actor
- computer/order UI
- money/shop system

Do not ask an agent to build the whole game in one prompt.
