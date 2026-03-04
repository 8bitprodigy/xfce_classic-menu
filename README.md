# XFCE4 Classic-Menu

A GNOME2/MATE-style classic menu for XFCE4, implemented as a panel plugin.

## What's implemented

- **Applications** — fully populated from the system's XDG application menu via garcon, organized by category. The Settings and System categories are intentionally excluded here and surfaced under the System menu instead.
- **Places** — mirrors your Thunar sidebar bookmarks exactly, by reading the same GTK bookmarks file Thunar uses. Includes Home, your personal bookmarks, Computer, Network, and Connect to Server.
- **System** — Settings Manager, Preferences and Administration submenus (also populated via garcon), Help, About XFCE, Lock Screen, and Log Out.

## Building

```shell
make
sudo make install
xfce4-panel --restart
```

## License

This code is released into the public domain.
Since not all jurisdictions recognize the public domain, it is also made available under the terms of the 0BSD license. See the [`LICENSE`](LICENSE) file for details.
