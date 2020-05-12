Download the latest version: https://github.com/rdoeffinger/xwa_ddraw_d3d11/releases/latest
The file you want is xwing_ddraw_d3d11_(version).zip

This is an implementation of ddraw.dll using Direct3D 11.
It is intended to be used with the X-Wing Alliance game.
This version is extended to also work with the other games in
the series, TIE95, X-Wing95, X-Wing vs TIE and Balance of Power.
More info at xwaupgrade.com:
http://www.xwaupgrade.com/phpBB3008/viewtopic.php?f=10&t=11167
It also works with the other games from the series:
X-Wing95, TIE95, X-Wing vs. TIE Fighter and Balance of Power


*** Requirements ***

This dll requires:
- DirectX 11
- a CPU with SSE2 support

How to install the latest version of DirectX
http://support.microsoft.com/kb/179113/en


*** Usage ***

To use this dll:
- place ddraw.dll, nopowervr.txt and ddraw.cfg next to (depending on game)
xwingalliance.exe, z_xvt__.exe, XWING95.exe or TIE95.exe

ddraw.cfg is a config file. Open it with a text editor for more details.
If you renamed the .exe files mentioned above, some features
may not work as expected.


Note for X-Wing vs. TIE Fighter:
In graphics configuration set "MIP Mapping" to "Blurry" for much
better texture at higher resolutions.
"Blurry" and "Noisy" are actually swapped compared to what the
option really does, thus setting to blurry makes textures sharper...

Note for XWING95 and TIE95:
Software cursor should be enabled automatically.
In case it does not work, it can also be enabled in the launcher, by adding
/softwarecursor to the XWING95.exe or TIE95.exe command line
or by running swcursor.bat from xwahacker (http://xwahacker.sf.net).

Note for TIE95:
Fix for ISD lasers not working is enabled by default.
Check ddraw.cfg AutoPatch option if you do not want it.

Note for X-Wing Alliance:
By default, the game supports only the following 4:3 resolutions:
- 640 x 480
- 800 x 600
- 1024 x 768
- 1152 x 864
- 1280 x 1024
- 1600 x 1200

You can replace these resolutions (and other things) with XWA Hacker by Reimar Döffinger (http://sourceforge.net/projects/xwahacker/files/).


*** License ***

xwa_ddraw_d3d11 is licensed under the MIT license. See LICENSE.txt


*** Source code ***

The source code of xwa_ddraw_d3d11 is situated at:
https://github.com/JeremyAnsel/xwa_ddraw_d3d11

This version was created from a fork at:
https://github.com/rdoeffinger/xwa_ddraw_d3d11


*** Credits ***

- Jérémy Ansel (JeremyaFr)
