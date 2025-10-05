<picture>
  <source media="(prefers-color-scheme: dark)" srcset="./press-kit/netfox-banner-hor.svg.preview.png">
  <source media="(prefers-color-scheme: light)" srcset="./press-kit/netfox-banner-hor-alt.svg.preview.png">
  <img alt="netfox banner" src="./docs/assets/press-kit/netfox-banner-hor-alt.svg.preview.png">
</picture>

# 🦊⚡ netfox boost

This addon is an addon for implementing rollback netcode in Godot. This is based on the [netfox](https://github.com/foxssake/netfox), and it uses a GDExtension that boosts the performance by running core functionality in C++. This project aims to be a drop-in replacement for the original netfox addon.

## Installation

Download the `netfox-boost` zip from the latest [GitHub action](https://github.com/Atlinx/netfox-boost/actions) or from the [Releases](https://github.com/Atlinx/netfox-boost/releases) page. Extract the contents and add them to the `res://addons/` directory of your project. 
If you already have netfox installed, make sure to remove the `netfox.internals` addon because its functionality is baked into the netfox-boost GDExtension.

### Supported versions

This addon supports Godot 4.5 and up. Some of the GDExtension API may be missing in older 4.x versions, which can cause the addon to not work.

## Development

After cloning your own copy to your local machine, make sure to initialize the godot-cpp git submodule via `git submodule update --init`.

## Generating Docs

To generate documentation, run the following command inside the `demo` folder.

```shell
godot --doctool ../ --gdextension-docs
```

### Configuring an IDE 
You can develop your own extension with any text editor and by invoking scons on the command line, but if you want to work with an IDE (Integrated Development Environment), you can use a compilation database file called `compile_commands.json`. Most IDEs should automatically identify this file, and self-configure appropriately.
To generate the database file, you can run one of the following commands in the project root directory:
```shell
# Generate compile_commands.json while compiling
scons compiledb=yes

# Generate compile_commands.json without compiling
scons compiledb=yes compile_commands.json
```