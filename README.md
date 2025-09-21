# dotfiles
<img width="1920" height="1080" alt="screenshot" src="https://github.com/user-attachments/assets/9c8095ba-bdad-4fad-86d8-306704b488fa" />

Configuration files, scripts, and customizations from my Arch Linux + dwm 6.5 setup.


## Structure

- **suckless/** - dwm, st, slock source trees with modifications
- **config/** - application configurations (dunst, rofi, picom, etc.)
- **scripts/** - custom shell scripts and utilities
- **firefox-floorp/** - browser UI customizations
- **fonts/** - font files and references
- **wallpapers-images/** - wallpapers and assets

## Suckless Tools

The `suckless/` directory contains complete source trees for:
- **dwm** - dynamic window manager
- **st** - simple terminal 
- **slock** - screen locker

### Important Note

Patch files are not included in this repository. To see what modifications were made:

```bash
# Clone vanilla suckless source
git clone https://git.suckless.org/dwm
git clone https://git.suckless.org/st  
git clone https://git.suckless.org/slock

# Compare with my configs
diff -u dwm/config.def.h suckless/dwm/config.h
diff -u st/config.def.h suckless/st/config.h
diff -u slock/config.def.h suckless/slock/config.h
```

## Disclaimer

This repository is shared for reference and inspiration. These configurations are tailored to my specific setup and workflow. **Do not blindly copy and paste** - review, understand, and adapt to your own needs.

## Usage

Feel free to browse, take inspiration, or borrow specific configurations. Each tool's config can be studied independently.
