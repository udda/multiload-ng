## Data providers

This directory contains one `.c` and one `.h` file for every **provider**.

A **provider** is an implementation of MlGraphTypeInterface. It contains:

- name (internal) and label (human readable)
- base hue (used by theme engine)
- dataset mode, see **core/dataset.h**
- number of dataset columns, each being a new color in the graph
- several callback functions (see **graph/type.h**)


### How to add a new provider

Here are the steps required to add a new provider, and make it available from
external API:

#### Step 1: Create provider sources

Create new provider sources in this directory (eg. `example.c` and `example.h`),
using another provider as skeleton. Implement any required callback.

- Remember to set correct `.n_data` and `.dataset_mode` in the interface
- Remember to choose a base hue (`.hue`) that is not used by other providers


#### Step 2: Add new graph type into global list

You have to:

1. Add new graph type in **graph/type.h** (eg. `ML_GRAPH_TYPE_EXAMPLE`)
2. Add new graph type in **graph/type.c** (eg. `CASE_TYPE_TO_IFACE(EXAMPLE)`)

#### Step 3: Translations

Any translatable string should be enclosed into a gettext macro, eg. `_("Traslate me")`
of `N_("Translate me")` for static content.

At least graph type's label should be translatable (`.label` field).
Add `src/provider/example.c` to **POTFILES.in**.

#### Step 4: Add files

You need to include your files into some lists to make *libmultiload* aware of
their existence.

1. **multiload.h**: add line `#include "provider/example.h"`
2. **Makefile.am**: add line `src/provider/example.c src/provider/example.h`
3. `git add example.c example.h`

#### Step 5: Test

Test the new provider adding a graph that uses it, and check whether it returns
expected data. If not, fix the callback functions and test again.


### Already used hues

This (hopefully up-to-date) table contains base hue value for each graph type.

| Hue  | Graph type                     |
| :--- | :----------------------------- |
| 0    | loadavg, loadavg_full          |
| 17   | javascript                     |
| 31   | diskio                         |
| 40   | testfail                       |
| 53   | net                            |
| 73   | entropy                        |
| 94   | battery                        |
| 113  | threads                        |
| 130  | sockets                        |
| 140  | wifi                           |
| 151  | ram                            |
| 164  | cpufreq                        |
| 178  | random                         |
| 196  | cpu                            |
| 210  | ping                           |
| 224  | storage                        |
| 249  | procrate                       |
| 261  | intrrate                       |
| 278  | swap                           |
| 293  | parametric                     |
| 310  | temperature                    |
| 335  | ctxtrate                       |
