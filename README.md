# 🦊⚡ netfox boost

This repository contains a GDExtension that boosts the performance of [netfox](https://github.com/foxssake/netfox) by running
repetitive tasks in cpp.

## Installation

Download the latest files from the releases page.

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