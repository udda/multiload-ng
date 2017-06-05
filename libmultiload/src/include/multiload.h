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
 * \file		multiload.h
 * \brief		Include file for libmultiload
 * \author		Mario Cianciolo
 * \copyright	(C) 2017 Mario Cianciolo (mr.udda at gmail.com)
 * \version		2.0
 *
 * This header allows to use libmultiload in your application, drawing
 * graphs of system resources.
 *
 */


#ifndef MULTILOAD__H__INCLUDED
#define MULTILOAD__H__INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !defined __GNUC__ && !defined __clang__
#define __attribute__(x) /*no-op*/
#endif

#ifdef  __cplusplus
extern "C" {
#endif


/**
 * \brief	Opaque type of Multiload
 *
 * This opaque structure holds internal representation of a Multiload container
 * and its elements (graphs and separators), along with all dimensions and positions.
 * Keeps track of every graph's data, style and config. It takes care of the timers
 * that trigger data retrieval for graphs.
 *
 * Multiload structure is a *façade* that exposes the internals of libmultiload
 * through a simple API.
 */
typedef struct _Multiload Multiload;

/**
 * \brief	Information about graph types
 *
 * This structure contains every useful information about a graph type. Use this
 * to build UI controls.
 *
 * \since	2.0
 * \see		multiload_list_graph_types
 */
typedef struct {
	const char *name;			///< Internal name (to be passed to multiload_add_graph())
	const char *label;			///< Human-readable name
	const char *description;	///< Human-readable description, may contain HTML
	const char *helptext;		///< Human-readable additional help text, may contain HTML
} MultiloadGraphType;

/**
 * \brief	Information about graph config keys
 *
 * This structure contains every useful information about a config entry. Use this
 * to build UI controls.
 *
 * An array of MultiloadConfigEntry structures for a graph can be obtained with
 * multiload_graph_list_config_entries()
 *
 * Key type is one of the following:
 * - \c 'i': integer, 32 bit (can have bounds, see below)
 * - \c 'I': integer, 64 bit (can have bounds, see below)
 * - \c 't': tristate (can be \c on, \c off or \c auto)
 * - \c 'b': boolean (can be \c true or \c false)
 * - \c 's': string (any string)
 * - \c 'l': list of strings (separated by \c |, eg. \c "a|b|c" is a three-elements list)
 * - \c '?': type error (this should not happen)
 *
 * \since	2.0
 * \see		multiload_graph_list_config_entries
 */
typedef struct {
	const char *key;			///< Key internal name
	char type;					///< Key type (\c 'i', \c 'I', \c 'b', \c 's', \c 'l', \c '?')

	const char *label;			///< Human-readable name
	const char *description;	///< Human-readable description, may contain HTML

	bool has_bounds;			///< \c true if bounds are set (only one of the following arrays)
	int64_t bounds[2];			///< Bounds for values of type \c 'i' or \c 'I' (integer)
} MultiloadConfigEntry;

/**
 * \brief	Textual information provided by graphs
 *
 * This structure contains textual data. Often this is a human-readable
 * representation of graph data, for use eg. in tooltips.
 *
 * Textual information is subdivided in *sections*. The meaning of Header,
 * Body and Footer is pretty straightforward. Some graphs also provide a
 * Table section. \c table_data is a two-dimensional array of NULL-terminated
 * strings\c char*, with 3 rows and 2 columns. Access single values using
 * \c table_data[row][col].
 *
 * Every graph decides what to put into each section. Some section may
 * even be empty (they will be \c NULL), so check for that before using.
 *
 * \since	2.0
 * \see		multiload_graph_get_caption
 */
typedef struct {
	const char *header;				///< Header section, usually contains data summary
	const char *body;				///< Body section, usually contains main information in a detailed fashion
	const char *footer;				///< Footer section, usually contains additional information

	const char *table_data[3][2];	///< Data for Table section
} MultiloadGraphCaption;

/**
 * \brief		Function type to be used as failure handler
 * \param[in]	ml Multiload that contains failed graph
 * \param[in]	index position of failed graph
 * \param[in]	error_code error code, graph-specific
 * \param[in]	error_msg localized error message
 * \param[in]	user_data user provided pointer, assigned with multiload_set_failure_handler()
 * \since		2.0
 *
 * Failure handler is a function that will be called every time a graph
 * fails. Multiload-ng will provide to the function a error code
 * and a localized error message.
 *
 * Set failure handler with multiload_set_failure_handler().
 *
 * \see			multiload_set_failure_handler
 */
typedef void (*MultiloadGraphFailureHandler)(Multiload *ml, int index, int error_code, const char *error_msg, void* user_data);

/**
 * \name Constructor / destructor
 *
 * These functions are used to create a new Multiload, and destroy it when done.
 *
 * Multiload are independent each other. Theoretically, although it's of little use,
 * an application could have multiple Multiload around.
 * @{
 */

/**
 * \brief		Create a new Multiload
 * \param[in]	size container size (see \ref terminology)
 * \param[in]	orientation container orientation (see \ref terminology)
 * \param[in]	padding container padding (see \ref terminology)
 * \return		a newly created Multiload
 * \retval		NULL if any error occours
 * \since		2.0
 *
 * This function creates a new Multiload, that can then be used as argument for
 * every other function. Destroy the Multiload when done, with multiload_destroy().
 *
 * \see			multiload_destroy
 */
Multiload *
multiload_new (int size, char *orientation, int padding)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Create a new Multiload using a JSON configuration from a string
 * \param[in]	json_def JSON string containing complete Multiload configuration
 * \return		a newly created Multiload
 * \retval		NULL if any error occours
 * \since		2.0
 *
 * This function creates a new Multiload, that can then be used as argument for
 * every other function. Destroy the Multiload when done, with multiload_destroy().
 *
 * \see			multiload_to_json
 * \see			multiload_new_from_json_file
 * \see			multiload_destroy
 */
Multiload *
multiload_new_from_json (const char *json_def)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Create a new Multiload using a JSON configuration from a file
 * \param[in]	json_def path of a file containing a JSON string with Multiload configuration
 * \return		a newly created Multiload
 * \retval		NULL if any error occours
 * \since		2.0
 *
 * This function creates a new Multiload, that can then be used as argument for
 * every other function. Destroy the Multiload when done, with multiload_destroy().
 *
 * \see			multiload_to_json_file
 * \see			multiload_new_from_json
 * \see			multiload_destroy
 */
Multiload *
multiload_new_from_json_file (const char *path)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Destroy a Multiload
 * \param[in]	ml Multiload to destroy
 * \since		2.0
 *
 * This function destroys a Multiload created with multiload_new(). Remember to
 * use this function to avoid memory leaks.
 *
 * \see			multiload_new
 */
void
multiload_destroy (Multiload *ml)
__attribute__(( __visibility__("default") ));


/**
 * \brief		Calculate memory footprint of a Multiload
 * \param[in]	ml Multiload
 * \return		total memory used by this Multiload, in bytes
 * \since		2.0
 *
 * This function calculates the exact number of total bytes used by Multiload
 * and its internal components.
 *
 * \note		The memory usage of a Multiload is less than the total memory
 *				footprint of the library.\n
 *				There is some (little) space used by global variables and
 *				threads stacks, that is not counted here.
 *
 * \see			multiload_new
 */
size_t
multiload_sizeof (Multiload *ml)
__attribute__(( __visibility__("default"), __pure__ ));
/// @}


/**
 * \name Container parameters
 *
 * Use these functions to set parameters of the container:
 * size, padding, orientation (see \ref terminology)
 * @{
 */

/**
 * \brief		Get common size of Multiload container
 * \param[in]	ml Multiload
 * \return		size of the elements' common direction, in pixels
 * \retval		-1 if operation fails
 * \since		2.0
 * \see			multiload_set_size
 */
int
multiload_get_size (Multiload *ml)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Set common size of Multiload container
 * \param[in]	ml Multiload
 * \param[in]	newsize size of elements' common direction
 * \retval		true if operation succeeds
 * \retval		false if operation fails
 * \since		2.0
 * \see			multiload_get_size
 */
bool
multiload_set_size (Multiload *ml, int newsize)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Get total size of all elements of Multiload container
 * \param[in]	ml Multiload
 * \return		sum of all elements' sizes, in pixels
 * \retval		-1 if operation fails
 * \since		2.0
 * \see			multiload_get_size
 */
int
multiload_get_length (Multiload *ml)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Get padding of a Multiload
 * \param[in]	ml Multiload
 * \return		padding of the elements, along the common direction
 * \retval		-1 if operation fails
 * \since		2.0
 * \see			multiload_set_padding
 */
int
multiload_get_padding (Multiload *ml)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Set padding of a Multiload
 * \param[in]	ml Multiload
 * \param[in]	newpadding padding of the elements, along the common direction
 * \retval		true if operation succeeds
 * \retval		false if operation fails
 * \since		2.0
 * \see			multiload_get_padding
 */
bool
multiload_set_padding (Multiload *ml, int newpadding)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Get orientation of a Multiload
 * \param[in]	ml Multiload
 * \return		orientation of Multiload
 * \retval		NULL if operation fails
 * \since		2.0
 * \see			multiload_set_orientation
 */
const char *
multiload_get_orientation (Multiload *ml)
__attribute__(( __visibility__("default"), __pure__ ));

/**
 * \brief		Set orientation of a Multiload
 * \param[in]	ml Multiload
 * \param[in]	orientation orientation of Multiload
 * \retval		true if operation succeeds
 * \retval		false if operation fails
 * \since		2.0
 * \see			multiload_get_orientation
 */
bool
multiload_set_orientation (Multiload *ml, char *orientation)
__attribute__(( __visibility__("default") ));

/// @}


/**
 * \name Elements
 *
 * Multiload containers embed child elements. They can be graphs or separators.
 *
 * These functions are used to manage creation, destruction and position of Multiload elements.
 * @{
 */

/**
 * \brief		Create a new graph and add it to a Multiload
 * \param[in]	ml Multiload
 * \param[in]	graph_type type of graph (internal name, see below)
 * \param[in]	size size of graph
 * \param[in]	border_size graph's border width
 * \param[in]	interval_ms milliseconds between updates of graph data
 * \param[in]	index desired position of new graph in the Multiload, or \c -1 for insert at the end
 * \return		actual index of inserted element
 * \retval		-1 if any error occours
 * \since		2.0
 *
 * This function creates a new graph with default style and config, and adds it
 * to \c Multiload \a ml.
 *
 * The newly created graph is already started. Graph can be stopped with multiload_graph_stop().
 *
 * \a graph_type can be obtained from multiload_list_graph_types(), reading
 * \c name field of MultiloadGraphTypeInfo structures.
 *
 * \see			multiload_element_is_graph
 * \see			multiload_list_graph_types
 */
int
multiload_add_graph (Multiload *ml, const char *graph_type, int size, int border_size, int interval_ms, int index)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Create a new separator and add it to a Multiload
 * \param[in]	ml Multiload
 * \param[in]	size size of separator
 * \param[in]	index desired position of new separator in the Multiload, or \c -1 for insert at the end
 * \return		actual index of inserted element
 * \retval		-1 if any error occours
 * \since		2.0
 * \see			multiload_element_is_separator
 */
int
multiload_add_separator (Multiload *ml, int size, int index)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Destroy an element (graph or separator)
 * \param[in]	ml Multiload
 * \param[in]	index position of element to destroy
 * \retval		true if operation succeeds
 * \retval		false if operation fails
 * \since		2.0
 * \see			multiload_add_graph
 * \see			multiload_add_separator
 */
bool
multiload_element_remove (Multiload *ml, int index)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Retrieve number of elements of a Multiload
 * \param[in]	ml Multiload
 * \return  	number of elements
 * \retval		-1 if operation fails
 * \since		2.0
 * \see			multiload_element_is_separator
 * \see			multiload_element_is_graph
 * \see			multiload_add_separator
 * \see			multiload_add_graph
 */
int
multiload_get_element_count (Multiload *ml)
__attribute__(( __visibility__("default"), __pure__ ));

/**
 * \brief		Get index of the element at given coordinates
 * \param[in]	ml Multiload
 * \param[in]	x X coordinate of the point to check
 * \param[in]	y Y coordinate of the point to check
 * \return  	index of element at (x, y)
 * \retval		-1 if there is no element at (x, y), or if any error occours
 * \since		2.0
 *
 * This function can be used for identifying the graph clicked by the user,
 * for tooltips and other mouse/touch related purposes.
 *
 * It may return \c -1 if the given coordinates point outside the surface or if
 * they point into the padding space.
 */
int
multiload_get_element_index_at_coords (Multiload *ml, int x, int y)
__attribute__(( __visibility__("default"), __pure__ ));

/**
 * \brief		Check whether a Multiload element is a graph
 * \param[in]	ml Multiload
 * \param[in]	index element position in Multiload
 * \retval		true if element at position \a index is a graph
 * \retval		false if element at position \a index is not a graph
 * \since		2.0
 * \see			multiload_add_graph
 */
bool
multiload_element_is_graph (Multiload *ml, int index)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Check whether a Multiload element is a separator
 * \param[in]	ml Multiload
 * \param[in]	index element position in Multiload
 * \retval		true if element at position \a index is a separator
 * \retval		false if element at position \a index is not a separator
 * \since		2.0
 * \see			multiload_add_separator
 */
bool
multiload_element_is_separator (Multiload *ml, int index)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Move an element a position backwards
 * \param[in]	ml Multiload
 * \param[in]	index original position of element
 * \retval		true if operation succeeds
 * \retval		false if operation fails (or if the elemens is in the first position)
 * \since		2.0
 */
bool
multiload_element_move_back (Multiload *ml, int index)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Move an element a position forward
 * \param[in]	ml Multiload
 * \param[in]	index original position of element
 * \retval		true if operation succeeds
 * \retval		false if operation fails (or if the elemens is in the last position)
 * \since		2.0
 */
bool
multiload_element_move_forward (Multiload *ml, int index)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Move an element into a new position
 * \param[in]	ml Multiload
 * \param[in]	index original position of element
 * \param[in]	new_index desired position of element
 * \retval		true if operation succeeds
 * \retval		false if operation fails
 * \since		2.0
 */
bool
multiload_element_move_to (Multiload *ml, int index, int new_index)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Get size of an element (graph or separator)
 * \param[in]	ml Multiload
 * \param[in]	index position of element
 * \return		size of element, in pixels
 * \retval		-1 if any error occours
 * \since		2.0
 * \see			multiload_element_set_size
 */
int
multiload_element_get_size (Multiload *ml, int index)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Resize an element
 * \param[in]	ml Multiload
 * \param[in]	index position of element
 * \param[in]	newsize new size of element
 * \retval		true if operation succeeds
 * \retval		false if operation fails
 * \since		2.0
 *
 * \a newsize must be greater than zero. If the element is a graph, \a newsize
 * must be big enough to allow the graph to draw itself (taking in account eg. border size)
 *
 * \see			multiload_element_get_size
 */
bool
multiload_element_set_size (Multiload *ml, int index, int newsize)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Get name of an element
 * \param[in]	ml Multiload
 * \param[in]	index position of element
 * \return		internal name of element type
 * \retval		NULL if any error occours
 * \since		2.0
 *
 * This function retrieves the internal type name of the given element. This name
 * is the same for all elements of the same type.
 * The type name is a constant lowercase string without spaces.
 *
 * \see			multiload_element_get_label
 */
const char *
multiload_element_get_name (Multiload *ml, int index)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Get label of a graph element
 * \param[in]	ml Multiload
 * \param[in]	index position of graph element
 * \return		localized label of graph
 * \retval		NULL if any error occours
 * \since		2.0
 *
 * This function retrieves the type label of the given element. This label is the
 * human-readable version of the internal type name returned by multiload_element_get_name().
 *
 * Unlike internal type name, element label is a longer, localized string, often
 * containing spaces.
 *
 * \see			multiload_element_get_name
 */
const char *
multiload_element_get_label (Multiload *ml, int index)
__attribute__(( __visibility__("default") ));

/// @}


/**
 * \name Graph elements
 *
 * These functions are specific to elements of type *graph*.
 * @{
 */

/**
 * \brief		Set failure handler
 * \param[in]	ml Multiload
 * \param[in]	handler failure handler
 * \since		2.0
 *
 * Failure handler will be called every time a graph fails. Use this function to
 * set the failure handler.
 *
 * Default failure handler just displays a debug message, tipically on console.
 * Set \a handler to \c NULL to revert to default handler.
 *
 * \note		Do not use resource-intensive functions as failure handlers.
 * 				The handler is called in the graph thread, that means that
 * 				the graph must wait for the failure handler to complete
 * 				execution before proceeding.
 *
 * \see			MultiloadGraphFailureHandler
 */
void
multiload_set_failure_handler (Multiload *ml, MultiloadGraphFailureHandler handler, void* user_data)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Get the list of all possible graph types
 * \return		a \c array of MultiloadGraphType
 * \retval		NULL if any error occours
 * \since		2.0
 *
 * The array is terminated by a null element (with all bits set to zero).
 *
 * \note		The returned array is newly allocated. It must be freed after use.
 *
 * Here is a way to check the end of the array in a loop:
 *
 * \code{.c}
 * MultiloadGraphType * list = multiload_list_graph_types ();
 * if (list != NULL) {
 *     for (int i = 0; list[i].name != NULL; i++) {
 *	       // ...
 *     }
 * }
 * \endcode
 * \see			multiload_list_graph_types_json
 */
MultiloadGraphType *
multiload_list_graph_types ()
__attribute__(( __visibility__("default") ));

/**
 * \brief		Get the list of all possible graph types, in JSON format
 * \return		a JSON string with information about graph types
 * \retval		NULL if any error occours
 * \since		2.0
 *
 * Returned JSON can be viewed as an associative array,
 * where keys are graph types.
 *
 * \note		The returned string is newly allocated. It must be freed after use.
 *
 * \see			multiload_list_graph_types
 */
char *
multiload_list_graph_types_json ()
__attribute__(( __visibility__("default"), __malloc__ ));

/**
 * \brief		Get ceiling value of a graph element
 * \param[in]	ml Multiload
 * \param[in]	index position of graph element
 * \return		ceiling value of graph element
 * \retval		0 if any error occours (or if ceiling is not set)
 * \since		2.0
 *
 * Ceiling of a graph is the maximum value that will be drawn. If nonzero, instead
 * of calculating a max value from graph data, the max value is taken directly as
 * the ceiling.
 *
 * \note		Ceiling can be \c 0 (means *no fixed ceiling*), so this function
 *				has no reliable way of checking for errors, as errors also make
 *				this function return \c 0.
 */
uint32_t
multiload_graph_get_ceiling (Multiload *ml, int index)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Set ceiling value of a graph element
 * \param[in]	ml Multiload
 * \param[in]	index position of graph element
 * \param[in]	ceiling ceiling value of graph element
 * \retval		true if operation succeeds
 * \retval		false if operation fails
 * \since		2.0
 *
 * Ceiling of a graph is the maximum value that will be drawn. If nonzero, instead
 * of calculating a max value from graph data, the max value is taken directly as
 * the ceiling.
 *
 * Setting the ceiling to \c 0 (it's the default value) makes graph calculate max
 * value from collected data.
 */
bool
multiload_graph_set_ceiling (Multiload *ml, int index, uint32_t ceiling)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Get border size of a graph element
 * \param[in]	ml Multiload
 * \param[in]	index position of graph element
 * \return		size of the graph border. in pixels
 * \retval		-1 if any error occours
 * \since		2.0
 */
int
multiload_graph_get_border_size (Multiload *ml, int index)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Set border size of a graph element
 * \param[in]	ml Multiload
 * \param[in]	index position of graph element
 * \param[in]	border_size size of the border, in pixels
 * \retval		true if operation succeeds
 * \retval		false if operation fails
 * \since		2.0
 */
bool
multiload_graph_set_border_size (Multiload *ml, int index, int border_size)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Get collect statistics (success and fail count)
 * \param[in]	ml Multiload
 * \param[in]	index position of graph element
 * \param[out]	success_count number of succeeded data collect operations
 * \param[out]	fail_count number of failed data collect operations
 * \retval		true if operation succeeds
 * \retval		false if operation fails
 * \since		2.0
 *
 * This function can be used to obtain the number of data collect operations for a graph.
 *
 * The count is split into succeeded and failed operations. The total count is the sum of these two values.
 *
 * One or both arguments can be \c NULL, if code is not interested in that value.
 */
bool
multiload_graph_get_collect_stats (Multiload *ml, int index, int64_t *success_count, int64_t *fail_count)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Get status of last collect operation
 * \param[in]	ml Multiload
 * \param[in]	index position of graph element
 * \retval		true if last collect operation succeeds
 * \retval		false if last collect operation failed (or if this function fails)
 * \since		2.0
 *
 * This function can be used to monitor success or failure of last collect operation for a graph.
 */
bool
multiload_graph_last_collect_succeeded (Multiload *ml, int index)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Get current caption for a graph element, in a MultiloadGraphCaption structure
 * \param[in]	ml Multiload
 * \param[in]	index position of graph element
 * \param[out]	out address of allocated buffer where to store the caption
 * \retval		true if operation succeeds
 * \retval		false if operation fails (i.e. the specified element is not a graph)
 * \since		2.0
 *
 * This function fills a MultiloadGraphCaption structure previously allocated.
 * Strings contained into that structure can be used to populate, for example,
 * a tooltip.
 *
 * \warning		Do not free returned strings: they are owned internally by graphs.
 *				Freeing MultiloadGraphCaption structure is OK.
 * \see			multiload_graph_get_caption_json
 */
bool
multiload_graph_get_caption (Multiload *ml, int index, MultiloadGraphCaption *out)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Get current caption for a graph element, in a JSON string
 * \param[in]	ml Multiload
 * \param[in]	index position of graph element
 * \return		a newly allocated JSON string with graph caption elements
 * \retval		NULL if operation fails (i.e. the specified element is not a graph)
 * \since		2.0
 *
 * This function creates a JSON string with caption of a graph element.
 * Strings contained into the returned JSON object can be used to populate, for
 * example, a tooltip.
 *
 * \note		The returned string is newly allocated. It must be freed after use.
 * \see			multiload_graph_get_caption
 */
char *
multiload_graph_get_caption_json (Multiload *ml, int index)
__attribute__(( __visibility__("default"), __malloc__ ));

/// @}


/**
 * \name Config of graph elements
 *
 * Use these functions to read and write config items of elements of type *graph*.
 * @{
 */

/**
 * \brief		Get value for a config key of a graph element
 * \param[in]	ml Multiload
 * \param[in]	index position of graph element
 * \param[in]	key config key to read, must be an existing key for the graph
 * \return		value for \a key
 * \retval		NULL if any error occours
 * \since		2.0
 * \note		Remember that every config value type (including numbers, booleans
 *				and lists) is passed by string.
 *
 * This function reads graph config and returns the value corresponding to
 * specified key.
 */
const char *
multiload_graph_get_config (Multiload *ml, int index, const char *key)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Set value for a config key of a graph element
 * \param[in]	ml Multiload
 * \param[in]	index position of graph element
 * \param[in]	key config key to read, must be an existing key for the graph
 * \param[in]	value new value for the specified key, must match the key type
 * \retval		true if operation succeeds
 * \retval		false if operation fails
 * \since		2.0
 * \note		Remember that every config value type (including numbers, booleans
 *				and lists) is passed by string. Pay attention to pass the correct
 *				value type to \a value.
 */
bool
multiload_graph_set_config (Multiload *ml, int index, const char *key, const char *value)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Get the list of all config entries of a graph element
 * \param[in]	ml Multiload
 * \param[in]	index position of graph element
 * \return		a \c array of MultiloadConfigEntry
 * \retval		NULL if any error occours
 * \since		2.0
 * \note		The returned array is newly allocated, but its elements belong
 *				to the library. Array (just the array) must be freed after use.
 *
 * The array is terminated by a null element (all bits are zero). The following
 * is a way to check the end of the array in a loop:
 *
 * \code{.c}
 * Multiload *ml;  // already initialized
 *
 * MultiloadConfigEntry * entries = multiload_graph_list_config_entries (ml, graph_index);
 * if (entries != NULL) {
 *     for (int i = 0; entries[i].type != '\0'; i++) {
 *	       // ...
 *     }
 * }
 *
 * // remember to clean up
 * free (entries);
 * \endcode
 *
 * \note		This list changes only when adding or removing graph elements,
 *				otherwise its contents are remain constant. There is no need
 *				to repeatedly call this function to update the interface.
 * \see			multiload_graph_list_config_entries_json
 * \see			MultiloadConfigEntry
 */
MultiloadConfigEntry *
multiload_graph_list_config_entries (Multiload *ml, int index)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Get the list of all config entries of a graph element, in JSON format
 * \param[in]	ml Multiload
 * \param[in]	index position of graph element
 * \return		a JSON string
 * \retval		NULL if any error occours
 * \since		2.0
 * \note		The returned string is newly allocated. It must be freed after use.
 *
 * Format and fields of returned JSON data are pretty similar to those returned
 * by multiload_graph_list_config_entries(). See \ref MultiloadConfigEntry for
 * additional details.
 *
 * \note		This list changes only when adding or removing graph elements,
 *				otherwise its contents are remain constant. There is no need
 *				to repeatedly call this function to update the interface.
 * \see			multiload_graph_list_config_entries
 * \see			MultiloadConfigEntry
 */
char *
multiload_graph_list_config_entries_json (Multiload *ml, int index)
__attribute__(( __visibility__("default"), __malloc__ ));

/// @}


/**
 * \name Style of graph elements
 *
 * Use these functions to read and change the style of elements of type *graph*.
 * @{
 */

/**
 * \brief		Retrieve graph style in JSON format
 * \param[in]	ml Multiload
 * \param[in]	index position of the graph element
 * \return		JSON-encoded graph style
 * \retval		NULL if operation fails
 * \since		2.0
 *
 * This function returns the style (colors) of the graph at given index.
 * Returned JSON data is a substring of JSON data returned by multiload_to_json().
 *
 * \see			multiload_graph_set_style_json
 * \see			multiload_to_json
 */
char *
multiload_graph_get_style_json (Multiload *ml, int index)
__attribute__(( __visibility__("default"), __malloc__ ));

/**
 * \brief		Apply JSON style to a graph
 * \param[in]	ml Multiload
 * \param[in]	index position of the graph element
 * \param[in]	style_json JSON-encoded graph style
 * \retval		true if operation succeeds
 * \retval		false if operation fails
 * \since		2.0
 *
 * This function takes the graph style contained in provided JSON string,
 * and applies it to the graph at given index.
 *
 * \a style_json can be obtained with multiload_graph_get_style_json().
 *
 * \see			multiload_graph_get_style_json
 */
bool
multiload_graph_set_style_json (Multiload *ml, int index, const char * style_json)
__attribute__(( __visibility__("default") ));

/// @}


/**
 * \name Controlling the graphs
 *
 * Use these functions to start and stop the graphs.
 *
 * A newly created graph is in *running* state. When a graph is *running*,
 * it collects its data periodically, at a rate defined by the graph interval.
 *
 * A graph can be started and stopped at any time.
 *
 * @{
 */

/**
 * \brief		Start a graph
 * \param[in]	ml Multiload
 * \param[in]	index position of the graph element
 * \retval		true if operation succeeds (or if graph is already running)
 * \retval		false if operation fails
 * \since		2.0
 *
 * This function starts the internal timer of the graph at position index.
 */
bool
multiload_graph_start (Multiload *ml, int index)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Stop a graph
 * \param[in]	ml Multiload
 * \param[in]	index position of the graph element
 * \retval		true if operation succeeds (or if graph is already stopped)
 * \retval		false if operation fails
 * \since		2.0
 *
 * This function stops the internal timer of the graph at position index.
 */
bool
multiload_graph_stop (Multiload *ml, int index)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Check running status of a graph
 * \param[in]	ml Multiload
 * \param[in]	index position of the graph element
 * \retval		true if graph is running
 * \retval		false if graph is not running (or if this function fails)
 * \since		2.0
 */
bool
multiload_graph_is_running (Multiload *ml, int index)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Get update interval of a graph element
 * \param[in]	ml Multiload
 * \param[in]	index position of the graph element
 * \return		interval between two graph updates, in milliseconds
 * \retval		-1 if operation fails
 * \since		2.0
 */
int
multiload_graph_get_interval (Multiload *ml, int index)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Set update interval of a graph element
 * \param[in]	ml Multiload
 * \param[in]	index position of the graph element
 * \param[in]	interval_ms interval between two graph updates, in milliseconds
 * \retval		true if operation succeeds
 * \retval		false if operation fails
 * \since		2.0
 */
bool
multiload_graph_set_interval (Multiload *ml, int index, int interval_ms)
__attribute__(( __visibility__("default") ));

/// @}


/**
 * \name Waiting for new data
 *
 * Use these functions to wait for new data from Multiload-ng, that is,
 * when one or more graphs have collected data.
 *
 * Waiting for data can be done in two ways:
 *
 * - blocking mode, using multiload_wait_for_data()
 * - non blocking mode, using multiload_needs_redraw()
 *
 * @{
 */

/**
 * \brief		Wait for new data from any graph
 * \param[in]	ml Multiload
 * \since		2.0
 *
 * This function blocks the calling thread until any of the running graph elements
 * has new data. This can be used to trigger redraw or other actions, by putting into
 * a loop. (see \ref code_example "example code above")
 */
void
multiload_wait_for_data (Multiload *ml)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Checks whether a redraw is required
 * \param[in]	ml Multiload
 * \retval		true if a redraw is required
 * \retval		false if a redraw is not required (or if any error occours)
 * \since		2.0
 *
 * This function checks whether the graphs need to be redrawed, either because of
 * size/elements change or because some of the running graph elements has new data.\n
 * Unlike multiload_wait_for_data(), this function does not block the calling thread.
 * Instead, it returns immediately, indicating the dirty status with the return value.
 */
bool
multiload_needs_redraw (Multiload *ml)
__attribute__(( __visibility__("default") ));

/// @}


/**
 * \name Image output
 *
 * Use these functions to export current graphical Multiload state.
 * In other words, these functions generate images (*screenshots*) of all
 * graphs and separators.
 * @{
 */

/**
 * \brief		Export Multiload surface into a PNG in-memory buffer
 * \param[in]	ml Multiload
 * \param[out]	len_ptr pointer to a \c size_t variable
 * \param[in]	compression_level zlib-style compression level
 * \return		buffer with PNG data
 * \retval		NULL if any error occours
 * \since		2.0
 *
 * This function takes the current graphical state of a Multiload, and exports
 * it in PNG format into a in-memory buffer. \a len_ptr must point to a \c size_t
 * variable. In case of success, it will contain buffer size.
 *
 * Choose the compression level to balance final size and compression speed.
 * Following zlib standards, compression level goes from 0 to 9.
 *
 * - `0`: no compression
 * - `1`: fastest compression, larger file
 * - `6`: default compression
 * - `9`: best (slowest) compression, smallest file
 *
 * \see multiload_write_to_png_file
 */
uint8_t *
multiload_write_to_png_buffer (Multiload *ml, size_t *len_ptr, int compression_level)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Export Multiload surface into a PNG file
 * \param[in]	ml Multiload
 * \param[in]	filename path of output PNG file
 * \param[in]	compression_level zlib-style compression level
 * \retval		true if operation succeeds
 * \retval		false if operation fails
 * \since		2.0
 *
 * This function takes the current graphical state of a Multiload, and exports
 * it into a PNG file, whose path is specified by \a filename.
 *
 * Choose the compression level to balance final size and compression speed.
 * Following zlib standards, compression level goes from 0 to 9.
 *
 * - `0`: no compression
 * - `1`: fastest compression, larger file
 * - `6`: default compression
 * - `9`: best (slowest) compression, smallest file
 *
 * \see multiload_write_to_png_buffer
 */
bool
multiload_write_to_png_file (Multiload *ml, const char *filename, int compression_level)
__attribute__(( __visibility__("default") ));

/**
 * \brief		Get Multiload surface into a buffer
 * \param[in]	ml Multiload
 * \param[out]	width pointer to a \c int, where to store surface width
 * \param[out]	height pointer to a \c int, where to store surface height
 * \return		buffer with pixel data
 * \retval		NULL if any error occours
 * \since		2.0
 *
 * This function takes the current graphical state of a Multiload, and exports
 * it (actual pixel values) into a buffer. In case of success, \a width and
 * \a height will contain image size.
 *
 * Pixel data can be considered already *alpha premultiplied*, as every pixel
 * has alpha value of either 255 (fully opaque) or 0 (fully transparent).
 *
 * \warning		For performance reasons, the returned pointer is not a copy,
 *				that's the actual internal surface buffer.\n You are allowed to
 *				edit contents of the returned buffer (just remember that will be
 *				overwritten after every redraw), but do not attempt to free it.
 */
uint8_t *
multiload_get_surface_data (Multiload *ml, int *width, int *height)
__attribute__(( __visibility__("default") ));

/// @}


/**
 * \name Persistence
 *
 * Use these functions to load and store current Multiload configuration to a
 * string or a file, in order to use it at next application launch.
 * @{
 */

/**
 * \brief		Export Multiload configuration into a JSON string
 * \param[in]	ml Multiload
 * \return		a newly allocated JSON string
 * \retval		NULL if any error occours
 * \since		2.0
 *
 * This function takes the complete current Multiload configuration, and dumps
 * it into a newly allocated JSON string.
 *
 * The returned string can be used to create a new Multiload object with the
 * same elements, using multiload_new_from_json().
 *
 * \note		The returned string is newly allocated. It must be freed after use.
 * \see			multiload_new_from_json
 */
char *
multiload_to_json (Multiload *ml)
__attribute__(( __visibility__("default"), __malloc__ ));

/**
 * \brief		Export Multiload configuration into a file
 * \param[in]	ml Multiload
 * \retval		true if operation succeeds
 * \retval		false if operation fails
 * \since		2.0
 *
 * This function takes the complete current Multiload configuration, and dumps
 * it to a file containing JSON data.
 *
 * The created file can be used to create a new Multiload object with the
 * same elements, using multiload_new_from_json_file().
 *
 * \see			multiload_new_from_json_file
 */
bool
multiload_to_json_file (Multiload *ml, const char *path)
__attribute__(( __visibility__("default") ));

/// @}


/**
 * \name Troubleshooting
 *
 * Use these functions to obtain detailed info when searching the cause of a problem.
 * @{
 */

/**
 * \brief		Collect system and Multiload related information into a ZIP file
 * \param[in]	ml Multiload
 * \param[in]	filename path of output ZIP file
 * \param[in]	compression_level zlib compression level
 * \retval		true if operation succeeds
 * \retval		false if operation fails
 * \since		2.0
 *
 * This function collects as much data as it can and dumps it into a ZIP file,
 * that can be used for troubleshooting and remote debugging.
 *
 * This function is intended to retrieve all data needed by developers in
 * order to find out what's going on.
 *
 * \note		No personal information is collected, there are just system files
 * 				and environment data. You can always examine generated ZIP.
 */
bool
multiload_debug_to_zip (Multiload *ml, const char *filename, int compression_level)
__attribute__(( __visibility__("default") ));

/// @}


/**
 * \name Library information
 *
 * Use these functions to obtain information about libmultiload itself.
 * @{
 */

/**
 * \brief		Retrieve libmultiload version string
 * \return		libmultiload version string
 * \since		2.0
 *
 * This function just returns the version string of libmultiload.
 *
 * \note		Returned string belongs to libmultiload and must not be freed.
 */
const char *
multiload_version()
__attribute__(( __visibility__("default"), __const__ ));

/// @}


#ifdef  __cplusplus
}
#endif

#endif /* MULTILOAD__H__INCLUDED */
