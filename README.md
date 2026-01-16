# TLIP (Tasnimul's Little Image Processor)

TLIP is a lightweight, high-performance image processing tool written in C. It specializes in resizing and compressing JPEG images to meet specific dimension and file size constraints. The project includes a native CLI, a basic native GTK4 GUI, and a comprehensive Java Swing wrapper for advanced batch processing.

## Features

- **Smart Resizing**: Scale images to exact width and height specifications.
- **Size Optimization**: Compress images to strictly fit under a target file size (in KB).
- **Dual Modes**: 
  - **Individual**: Interactive processing for single files.
  - **Batch**: Process entire directories automatically.
- **Multiple Interfaces**:
  - **CLI**: Efficient command-line interface for scripts and automation.
  - **Native GUI**: Simple GTK4-based interface.
  - **Java Wrapper**: A robust Swing GUI for managing batch operations easily.

## Dependencies

Ensure you have the following installed on your Linux system:

- **Compiler**: `clang` (default) or `gcc`.
- **GTK4**: Development libraries (`libgtk-4-dev`).
- **TurboJPEG**: High-performance JPEG library (`libturbojpeg0-dev`).
- **Java JDK**: Required only for the Java GUI wrapper.

## Build Instructions

### 1. Build Core Application (C)
Compiles the main `tlip` binary.
```bash
make
```

### 2. Build Java GUI (Optional)
Compiles the Java Swing wrapper.
```bash
cd gui-swing
make
```

## Usage

### Native GUI
Run the program without arguments to launch the native GTK interface:
```bash
./tlip
```

### Command Line Interface (CLI)
Use the `--cli` flag to run in terminal mode.

**Syntax:**
```bash
./tlip --cli <input> <width> <height> <size_kb> <output_dir> <mode> [no-open]
```

- **input**: Path to a single image or a directory of images.
- **width**: Target width in pixels (use `""` to keep original).
- **height**: Target height in pixels (use `""` to keep original).
- **size_kb**: Maximum file size in KB (use `""` for no limit).
- **output_dir**: Destination directory for processed files.
- **mode**: 
  - `individual`: Interactive mode (prompts for settings per file).
  - `not-individual`: Batch mode (applies settings to all files).
- **no-open**: (Optional) Prevents opening the output image/folder after processing.

**Example:**
Resize `image.jpg` to 1920x1080, kept under 500KB, and save to `./out`:
```bash
./tlip --cli image.jpg 1920 1080 500 ./out not-individual
```

### Java GUI Wrapper
The Java wrapper provides a more user-friendly interface for batch operations.

```bash
cd gui-swing
make run
```
*(Note: requires the C `tlip` binary to be built in the root directory first)*
