# gitedit - Safe Git Commit Message Editor

A robust, self-contained C program that allows safe editing of Git commit messages at the raw object level. This tool provides an interactive interface for modifying commit messages while maintaining repository safety through automatic backups.

>[!WARNING]
> This tool modifies Git objects directly and can potentially break your repository. While gitedit includes safety features like automatic backups, editing Git history can have serious consequences, especially for collaborative repositories. Always create backups and understand the risks before using this tool.

## Features

- **Safe Editing**: Automatically creates backups of `.git` directory before any modifications
- **Interactive Interface**: User-friendly terminal interface for selecting and editing commits
- **Compression Support**: Uses miniz library for zlib-compatible Git object compression/decompression
- **Modular Design**: Clean separation of concerns with dedicated modules for different functionalities
- **Backup System**: Easy backup creation and restoration capabilities
- **Logging**: Comprehensive logging of all actions in `gitedit.log`
- **Branch Detection**: Automatically detects current Git branch
- **Tip Commit Detection**: Warns when editing non-tip commits (history-changing operations)

## Project Structure

```
gitedit/
├── src/
│   ├── main.c          # Entry point and main workflow
│   ├── utils.c         # Helper functions (file I/O, strings, logging)
│   ├── backup.c        # Git folder backup/restore functionality
│   ├── git_objects.c   # Git object reading/writing with compression
│   └── editor.c        # Interactive commit selection and editing
├── include/
│   ├── utils.h         # Utilities header
│   ├── backup.h        # Backup functions header
│   ├── git_objects.h   # Git object functions header
│   └── editor.h        # Editor functions header
├── deps/
│   └── miniz.h         # Single-file zlib-compatible compression library
├── bin/                # Compiled executable
├── Makefile            # Build configuration
└── README.md           # This documentation
```

## Requirements

- C17-compatible compiler (clang recommended)
- Git repository
- Unix-like environment (Linux, macOS, WSL)

## Building

### Prerequisites

Ensure you have clang installed:
```bash
# macOS
xcode-select --install

# Linux (Ubuntu/Debian)
sudo apt-get install clang

# Linux (CentOS/RHEL)
sudo yum install clang
```

### Compilation

```bash
# Clone or download the project
cd gitedit

# Build the executable
make

# Or build with debug flags
make debug

# Install system-wide (optional)
sudo make install

# Uninstall
sudo make uninstall
```

## Usage

### Basic Usage

```bash
# Run from within a Git repository
./bin/gitedit

# Or if installed system-wide
gitedit
```

### Command Line Options

```bash
# Show help
gitedit --help

# List available backups
gitedit --list

# Restore from backup
gitedit --restore
```

### Workflow

1. **Repository Check**: gitedit verifies you're in a Git repository
2. **Backup Creation**: Automatically creates a timestamped backup of `.git`
3. **Branch Detection**: Identifies the current branch
4. **Commit Listing**: Shows the last 10 commits with details
5. **Commit Selection**: Interactive selection of commit to edit
6. **Message Editing**: Multi-line message editor with preview
7. **Confirmation**: Shows changes and asks for final confirmation
8. **Object Update**: Recompresses and writes the modified commit object
9. **Reference Update**: Updates branch reference if editing tip commit

## Safety Features

### Automatic Backups

Before any modification, gitedit creates a complete backup of your `.git` directory:
```bash
.git_backup_20240329_143022/
```

### Tip Commit Detection

gitedit distinguishes between:
- **Tip commits**: Safe to edit, branch reference automatically updated
- **Non-tip commits**: History-changing, warns about potential issues

### Logging

All actions are logged to `gitedit.log`:
```
[2024-03-29 14:30:22] ACTION: BACKUP_CREATED - Created backup at .git_backup_20240329_143022
[2024-03-29 14:30:25] ACTION: COMMIT_EDITED - Edited commit a1b2c3d4 -> e5f6g7h8 on branch main
```

### Confirmation Prompts

Multiple confirmation points ensure you don't accidentally make changes:
- Commit selection confirmation
- Change preview confirmation
- Final apply confirmation

## Backup Management

### Listing Backups

```bash
gitedit --list
```

### Restoring Backups

```bash
gitedit --restore
# Follow prompts to select backup timestamp
```

### Manual Backup Management

Backups are stored as `.git_backup_TIMESTAMP` directories. You can manually:

```bash
# List backups
ls -la .git_backup_*

# Remove old backups
rm -rf .git_backup_20240329_143022
```

## Advanced Usage

### Editing Non-Tip Commits

When editing commits that are not the current tip:
1. gitedit warns about history-changing nature
2. Creates the new commit object
3. Provides guidance for updating branch references

You may need to use commands like:
```bash
git reset --hard <new_sha>
# or
git update-ref refs/heads/main <new_sha>
```

### Debug Mode

Build with debug flags for enhanced error checking:
```bash
make debug
./bin/gitedit
```

## Troubleshooting

### Common Issues

1. **"Not in a Git repository"**
   - Ensure you're running gitedit from within a Git repository
   - Check that `.git` directory exists

2. **"Failed to create backup"**
   - Check disk space
   - Verify write permissions
   - Ensure `.git` directory is accessible

3. **"No commits found"**
   - Repository may be empty (no commits yet)
   - Check if you're on the correct branch

4. **Compilation errors**
   - Ensure clang is installed
   - Check C17 support
   - Verify all dependencies are present

### Recovery

If something goes wrong:
1. Use `gitedit --list` to see available backups
2. Use `gitedit --restore` to restore from backup
3. Or manually restore: `rm -rf .git && mv .git_backup_TIMESTAMP .git`

## Development

### Code Style

- C17 standard
- clang-compatible
- Modular design with clear separation of concerns
- Comprehensive error handling
- Memory-safe practices

### Dependencies

- **miniz**: Single-file zlib-compatible compression library
- **Standard C library**: All other functionality uses standard C

### Testing

```bash
# Test compilation
make test-compile

# Debug build with sanitizers
make debug

# Clean build artifacts
make clean
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make changes with proper error handling
4. Test thoroughly
5. Submit a pull request

## License

This project is provided as-is for educational and utility purposes. Use at your own risk.

## Acknowledgments

- **miniz**: Single-file zlib compression library by Rich Geldreich
- **Git**: For the object storage format that makes this tool possible