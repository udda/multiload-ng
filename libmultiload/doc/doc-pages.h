/*
 * Copyright (C) 2017 Mario Cianciolo <mr.udda@gmail.com>
 *
 * This file is part of Multiload-ng.
 * 
 * Multiload-ng is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Multiload-ng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Multiload-ng.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * \mainpage	Main page
 * 
 * Welcome to the documentation for libmultiload, the library backend of all
 * Multiload-ng applications and plugins.
 *
 * ## API Documentation
 *
 * Full API documentation is available \link multiload.h here \endlink.
 * 
 * \author		Mario Cianciolo
 * \copyright	(C) 2017 Mario Cianciolo (mr.udda at gmail.com)
 *
 * For installation instructions and other info, please refer to
 * the README file located in source code, or to the project
 * <a href="https://github.com/udda/multiload-ng">GitHub page</a>.
 */


/**
 * \page gettingstarted Getting started
 *
 * Multiload-ng can be embedded in virtually every application, because it has
 * no particular dependencies.
 * 
 * In addition to panel plugins and standalone applications, you can use libmultiload
 * into every application: feeding the graphs with your data, you can draw usage
 * of an internal resource of your application.
 *
 * \section prepare_the_sources Prepare the sources
 * 
 * In order to work with libmultiload, you just need to:
 * 1. add library to your compiler flags (`-lmultiload`)
 * 2. include header to source code (`# include <multiload.h>`)
 *
 * And you're all set. The recommended way to do both is to use **pkg-config**.
 * This solves most common issues, like missing flags or wrong paths.
 * 
 * Here is an example with *gcc*:
 *
 * `gcc $(pkg-config --libs --cflags libmultiload-2) source.c`
 *
 * \section usage_modes Usage modes
 *
 * Multiload-ng can be used in several modes:
 * - real-time: use multiload_wait_for_data() in a loop
 * - on demand: drawing is done only when needed, so you can request a "screenshot"
 *   of Multiload status at any time. Multiload-ng will continue to collect data
 *   without drawing it, using very few resources
 * - mixed (timed): implement a timer that periodically calls multiload_needs_redraw()
 *   and performs the drawing if the function returns \c true (think of it as
 *   changing the FPS)
 * 
 * Multiload-ng can also be used in **non-graphical** mode: use it only to collect
 * data (without drawing it), then read the data into your application and use
 * it as you need.
 * 
 * \section code_example Code example
 * 
 * Here is a quick code example. Here we are using Multiload-ng in *real-time*
 * mode, doing stuff every time there is new data to draw.
 *
 * \code{.c}
 * #include <multiload.h>
 *
 * int main() {
 *     // create Multiload
 *     Multiload *ml = multiload_new (...);
 *
 *     // in this example we add only one graph and one separator
 *     int graph_index      = multiload_create_graph (ml, ...);
 *     int separator_index  = multiload_create_separator (ml, ...);
 *
 *     // start the graph and check for errors
 *     if (graph_index < 0 || separator_index < 0) {
 *         // error, terminate the application
 *     }
 *
 *     // application main loop
 *     while (true) {
 *         multiload_wait_for_data (ml);
 *         // redraw or other action
 *     }
 *
 *     return 0;
 * }
 * \endcode
 */


/**
 * \page structure Structure of a Multiload
 *
 * A Multiload is basically a container. Its elements are arranged sequentially,
 * ordered by their **index**.
 *
 * A picture is better of a thousand words. Here's how the various sizes are related to each other:
 *
 * \image html structure_h.png "Structure of a horizontal Multiload"
 *
 * Just rotate the image 90° and you get the structure of a vertical Multiload.
 * All terms in the picture above are covered in detail in \ref terminology page.
 */


/**
 * \page terminology Terminology
 *
 * This page explains the terms used throughout the documentation.
 * 
 * \section term_multiload Multiload
 * This is the main object. You need to create one in order to work with the library.
 *
 * Multiload has an internal container (see below) that will host all your
 * graphs and separators.
 * 
 * Create a Multiload using multiload_new() or multiload_new_from_json().
 *
 * \section term_container Container
 * We call *Multiload container* (or just *container*) the *box* that will graphically
 * contain the elements (see \ref structure). A container is automatically created
 * in every Multiload, so you don't have to worry about its existence.
 *
 * \section term_element Element
 * We call *Multiload element* (or just *element*) a graphical object that will
 * be part of a Multiload. Elements can be **graphs** or **separators**.
 *
 * A Multiload element cannot exist on its own (technically it could, with
 * limited capabilities, but this API doesn't allow that). All elements are childs
 * of a Multiload container.
 *
 * To add and remove elements, use multiload_create_graph(),
 * multiload_create_separator() and multiload_destroy_element().
 * 
 * \section term_size Container size, Element size
 * Elements in a Multiload are arranged linearly, means that each element shares one
 * of its two dimensions with all other elements, and with the container itself.
 * 
 * The size of any element along the common dimension it's called **container size**.
 *
 * Consequently, the size of each element along the non-shared dimension, it's called
 * **element size**. 
 *
 * For example, in a Multiload with *horizontal* orientation, the container size is
 * the vertical length, while the element size is the horizontal length. To better
 * understand, take a look at the \ref structure page.
 *
 * \section term_padding Padding
 * This is the extra space on both edges of the container size. To better
 * understand, take a look at the \ref structure page.
 *
 * \section term_orientation Orientation
 * This describes how the elements are arranged inside the container. Orientation
 * can be \c "horizontal" or \c "vertical".
 *
 * Multiload orientation (see above) decides which dimension will be shared by
 * the elements:
 * - a *horizontal* container shares its height with the elements
 * - a *vertical* container shares its width with the elements
 * 
 * To better understand, take a look at the \ref structure page.
 *
 * \section term_surface Surface
 * It's the rectangular array of pixels on which Multiload-ng draws its elements.
 *
 * Info about the internal format are available in \ref surface page.
 * 
 */


/**
 * \page performance Performance and resources
 *
 * Graph model has undergone some major improvements over the previous versions
 * of Multiload-ng, that greatly improve performance.
 *
 * Each graph collects its data in a separate thread. This results
 * in better responsiveness: even a slow graph that calls external processes
 * cannot not slow down other graphs.
 *
 * Graph data is collected periodically (set the frequency with
 * multiload_set_graph_interval()), but actual drawing, which often is
 * the most CPU intensive task, is **deferred** until it's needed.
 * 
 * For example an application may choose to show the graphs at a slower
 * rate or only on demand (see \ref usage_modes).
 *
 * \section memory_footprint Memory footprint
 * Memory usage of libmultiload is very little.
 *
 * Most of the memory is used for graph surface buffers. This means that the
 * overall memory use grows roughly linearly with container width and height.\n
 * This also means that separators do not have a noticeable impact on memory.
 *
 * Total byte usage of a Multiload can be calculated at runtime using multiload_sizeof().
 * The returned number does not take in account shared data (less than 1 KB), but should
 * reflect overall total memory usage of the library,
 *
 * \section performance_hints Performance hints
 * Multiload-ng, like every application/library, uses system resources.
 * These are very well measured, and big efforts are done to keep them at lowest possible
 * levels, but like any monitoring app, it needs to read hundreds of files and parse thousands
 * lines of text, so there is a lower limit to that, and we're very close to it.
 * 
 * You can experience this by starting a process monitor (like `top`/`htop` or
 * `gnome-system-monitor`), you will soon notice that CPU usage raises by some amount.
 * Multiload-ng does all that these monitors do (and does much more), while consuming
 * **less** CPU cycles than the aforementioned applications.
 *
 * That said, here are some suggestions to further limit resources usage of Multiload-ng
 * in your machine:
 *
 * 1. Raise update intervals
 * 2. Avoid to add too many graphs
 * 3. Reduce surface size (total width and height)
 * 4. Avoid graphs that call external processes
 */


/**
 * \page surface Surfaces
 *
 * At some point when using libmultiload, you will need to get the "pixels" to
 * draw them in your application. This page explains how to get (and then use)
 * surface data (see \ref term_surface "here").
 *
 * \section Internals
 * Each graph draws its data on its own \ref term_surface "surface", that is a
 * rectangular array of pixels which are colored according to graph background,
 * border and data.
 * Graph surface has exactly the same dimensions (width, height) of underlying graph.
 *
 * Moreover, the \ref term_container "container" has its own surface, which is
 * used to draw the surfaces of its elements taking in account \ref term_padding "padding"
 * and separators.
 *
 * libmultiload provides access to container surface data (the whole image).
 * Tipically, that's all you need.
 * 
 * \section surface_memory_format In-memory surface format
 *
 * Simply put, a surface is a linear array of pixels. Each pixel
 * is a 32 bit ARGB, arranged as an integer with host machine
 * <a href="https://en.wikipedia.org/wiki/Endianness">endianness</a>.
 * This is the format that most drawing libraries expect.
 *
 * You can get a pointer to surface data with multiload_get_surface_data().
 *
 * Here's how the pixels are arranged in machines with different endianness:
 *
 * <table>
 * <tr>
 * <td></td>
 * <td colspan="4" style="text-align: center;">32 bit</td>
 * <td colspan="4" style="text-align: center;">32 bit</td>
 * <td colspan="4" style="text-align: center;">32 bit</td>
 * <td colspan="4" style="text-align: center;">32 bit</td>
 * </tr>
 *
 * <tr>
 * <td>Little endian</td>
 * <td>B</td><td>G</td><td>R</td><td>A</td>
 * <td>B</td><td>G</td><td>R</td><td>A</td>
 * <td>B</td><td>G</td><td>R</td><td>A</td>
 * <td>B</td><td>G</td><td>R</td><td>A</td>
 * <td>...</td>
 * </tr>
 * 
 * <tr>
 * <td>Big endian</td>
 * <td>A</td><td>R</td><td>G</td><td>B</td>
 * <td>A</td><td>R</td><td>G</td><td>B</td>
 * <td>A</td><td>R</td><td>G</td><td>B</td>
 * <td>A</td><td>R</td><td>G</td><td>B</td>
 * <td>...</td>
 * </tr>
 * 
 * <tr>
 * <td>PDP-endian</td>
 * <td>G</td><td>B</td><td>A</td><td>R</td>
 * <td>G</td><td>B</td><td>A</td><td>R</td>
 * <td>G</td><td>B</td><td>A</td><td>R</td>
 * <td>G</td><td>B</td><td>A</td><td>R</td>
 * <td>...</td>
 * </tr>
 * 
 * </table>
 * 
 */

/**
 * \page environment Environment variables
 *
 * Multiload-ng behavior is influenced by some environment variables.
 *
 * ### Why environment variables?
 * Because it's the only way of passing parameters that works with every type
 * of application (daemons, console apps, GUI apps, panel plugins...).
 * Using environment variables allows to set some parameters
 * in a consistent and format-agnostic way.
 *
 * \section env_variables Environment variables list
 * ### MULTILOAD_DEBUG
 * Used to select enabled debug levels.
 * More than one level can be specified, just separate them with commas.
 * Allowed values are:
 *
 * - `debug`
 * - `info`
 * - `warning`
 * - `error`
 * - `all` (equals to `debug,info,warning,error`)
 *
 * Default debug level is `warning,error`
 * 
 * ### MULTILOAD_DEBUG_FILE
 * Used to redirect debug output to a file. Specified path must be writable
 * by the application.
 *
 * By default, debug messages are printed to *stderr*. If specified file cannot
 * be created/written, output will fall back to *stderr*.
 * 
 */

/**
 * \page faq FAQ
 *
 * #### Which programming languages are supported by libmultiload?
 * Multiload-ng is fully written in C, so C itself is natively supported. Here is
 * a (hopefully incomplete) list of supported languages:
 *
 * - C
 * - C++
 * - D (use `extern(C) { # include <multiload.h> }`)
 *
 * Porting to other common languages like Java and Python is planned.
 * 
 */
