ADBUSB Schematics and PCB
-------------------------

This folder has a reference implementation of a board for ADBUSB. Feel free to copy it or modify it as you need. The important files in here are:

- schematic_mega32.sch: schematic
- board_mega32.pcb: pcb layout
- packages/: pcb footprints of the parts in the layout provided

The files were created using the gEDA software suite: gschem, pcb, and others. On Debian-based systems (including Ubuntu) you can get this with::

    % sudo apt-get install geda

Open up the schematic with ``gschem schematic_mega32.sch`` and the pcb with ``pcb board_mega32.pcb``. The pages that I had bookmarked when doing this layout (and that can help you learn gEDA) are:

* http://wiki.geda-project.org/geda:gsch2pcb_tutorial
* http://www.gedasymbols.org/
