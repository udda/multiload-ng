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


var Multiload = {

	// <editor-fold> Main properties

	get jqueryElement() {
		return $('#multiload-image');
	},

	constants: {
		elementSize:    { min: 4,    max: 1000,       default: 40    },
		graphBorder:    { min: 0,    max: 400,        default: 2     },
		graphCeiling:   { min: 0,    max: 4294967295, default: 0     },
		graphInterval:  { min: 10,   max: 10000000,   default: 800   },
	},

	// </editor-fold>

	// <editor-fold> Initialization
	init: function() {
		var ml = this.jqueryElement;

		ml.tooltip ({
			items: "img[alt]",
			track: true,
			show: true,
			hide: false,
			//position: { my: "left+15 center", at: "right center" },
		});

		ml.mousemove (function(e) {
			var x = e.pageX - $(this).offset().left;
			var y = e.pageY - $(this).offset().top;

			Multiload.hoverElement = Multiload.indexAtCoords(x, y);

			if (Multiload.hoverElement.is_graph === false)
				ml.tooltip({ hide: false });
		});

		ml.mouseenter (function() {
			Multiload.mouseOver = true;
		});

		ml.mouseleave (function() {
			Multiload.mouseOver = false;
			Multiload.hideTooltip();
		});

		// preload JSON data
		this.reload();

		// start update function
		this.updateInterval = 1000;
	},

	reload: function() {
		this.loadData();
		this.loadGraphTypes();
		this.loadLibVersion();
		this.loadLocaleData();
	},

	// </editor-fold>

	// <editor-fold> Periodic update
	_updateInterval: 0,
	_updateIntervalHandle: null,
	_updateFunc: function() {
		var status = Multiload._jsonCommand('status');
		if (status == null) {
			Multiload.jqueryElement.hide();
			$('#multiload-placeholder').show();
			return;
		} else {
			Multiload.jqueryElement.show();
			$('#multiload-placeholder').hide();
		}

		if (status.value == "dirty_data") {
			console.log ("Triggered data reload");
			Multiload.loadData();
		} else if (status.value == "reload") {
			console.log ("Triggered full reload");
			Multiload.reload();
		}

		// update image
		Multiload.jqueryElement.attr('src', "multiload.png?time="+new Date().getTime());
		Multiload.updateTooltip();
	},
	get updateInterval() {
		return this._updateInterval;
	},
	set updateInterval(ms) {
		if (this._updateIntervalHandle != null) {
			window.clearInterval (this._updateIntervalHandle);
			this._updateIntervalHandle = null;
		}

		if (ms == 0) {
			this._updateInterval = 0;
			return;
		}

		if (ms < 100)
			ms = 100;

		this._updateInterval = ms;
		this._updateIntervalHandle = window.setInterval (this._updateFunc, ms);
	},

	// </editor-fold>

	// <editor-fold> Data exchange with server

	logCommands: false,
	_jsonCommand: function(commandName, parameters) {
		// build query string
		var queryString = 'command/' + commandName;
		if (parameters)
			queryString += '?' + $.param(parameters);

		// server AJAX call
		var result = null;
		var error = null;
		$.ajax({
			url:       queryString,
			type:      'get',
			dataType:  'json',
			async:     false,
			cache:     false,
			success:   function(data) {
				result = data;
			},
			error:     function(jqXHR, textStatus, errorThrown) {
				error = {
					'jqXHR':        jqXHR,
					'textStatus':   textStatus,
					'errorThrown':  errorThrown
				}
			}
		});

		// log (when enabled)
		if (this.logCommands)
			console.debug ({
				'commandName':  commandName,
				'parameters':   parameters,
				'queryString':  queryString,
				'result':       result,
				'error':        error
			});

		// return
		return result;
	},

	libVersion: "",
	loadLibVersion: function() {
		var mlVersion = Multiload._jsonCommand('library-version');
		if (mlVersion) {
			this.libVersion = mlVersion.value;
			console.log ("Loaded library version: "+ this.libVersion);
		} else {
			console.error ("Cannot load library version");
		}
	},

	graphTypes: {},
	loadGraphTypes: function() {
		this.graphTypes = Multiload._jsonCommand('graph-types');
		if (this.graphTypes)
			console.log ("Loaded " + this.graphTypes.Length + " graph types");
		else
			console.error ("Cannot load graph types");
	},

	data: {},
	loadData: function() {
		var mlData = Multiload._jsonCommand('data');
		if (mlData) {
			this.data = mlData.container;
			console.log ("Loaded Multiload data");
		} else {
			console.error ("Cannot load Multiload data");
		}
	},

	getCaption: function(index) {
		return Multiload._jsonCommand('caption', {
			'index': index
		});
	},

	indexAtCoords: function(x,y) {
		return Multiload._jsonCommand('index-at-coords', {
			'x': x,
			'y': y,
		});
	},

	hasFile: function(x,y) {
		var ret = Multiload._jsonCommand('has-file');
		if (!ret)
			return false;

		return ret.value;
	},

	// </editor-fold>

	// <editor-fold> Elements manipulation
	createGraph: function(size, type, border, interval, position) {
		return Multiload._jsonCommand('create', {
			'type':       'graph',
			'size':        size,
			'graph-type':  type,
			'border':      border,
			'interval':    interval,
			'position':    position
		}).value;
	},
	createSeparator: function(size, position) {
		return Multiload._jsonCommand('create', {
			'type':       'separator',
			'size':        size,
			'position':    position
		}).value;
	},
	deleteElement: function(index) {
		return Multiload._jsonCommand('delete', {
			'index': index
		}).value;
	},
	moveElement: function(start, end) {
		if (start == end)
			return true;

		return Multiload._jsonCommand('move', {
			'from': start,
			'to':   end
		}).value;
	},
	pauseGraph: function(index) {
		return Multiload._jsonCommand('pause', {
			'index': index
		}).value;
	},
	resumeGraph: function(index) {
		return Multiload._jsonCommand('resume', {
			'index': index
		}).value;
	},
	setElementSize: function(index, value) {
		return Multiload._jsonCommand('set-element-size', {
			'index': index,
			'value': value
		}).value;
	},
	setGraphBorder: function(index, value) {
		return Multiload._jsonCommand('set-graph-border', {
			'index': index,
			'value': value
		}).value;
	},
	setGraphCeiling: function(index, value) {
		return Multiload._jsonCommand('set-graph-ceiling', {
			'index': index,
			'value': value
		}).value;
	},
	setGraphInterval: function(index, value) {
		return Multiload._jsonCommand('set-graph-interval', {
			'index': index,
			'value': value
		}).value;
	},

	// </editor-fold>

	// <editor-fold> Graph configuration
	getConfigEntries: function(index) {
		return Multiload._jsonCommand('config-entries', {
			'index': index
		});
	},

	getGraphConfig: function(index, key) {
		return Multiload._jsonCommand('get-config', {
			'index': index,
			'key'  : key
		});
	},

	setGraphConfig: function(index, key, value) {
		return Multiload._jsonCommand('set-config', {
			'index': index,
			'key'  : key,
			'value': value
		});
	},
	// </editor-fold>

	// <editor-fold> Tooltip
	mouseOver: false,
	hoverElement: -1,

	setTooltip: function(text) {
		this.jqueryElement.tooltip("option", "content", text);
		this.jqueryElement.tooltip("open");
	},

	updateTooltip: function() {
		if (Multiload.mouseOver) {
			var i = Multiload.hoverElement.index;
			if (i < 0)
				return;

			var element = this.data.elements[i];

			if (element.type == "graph") {
				var g = element.graph;
				var caption = Multiload.getCaption(i);

				if (caption) {
					Multiload.setTooltip(function(caption) {
						var ret = '<div class="multiload-tooltip">';

						ret += '<div class="title">' + Multiload.graphTypes[g.type].label + '</div>';

						if (caption.header)
							ret += '<div class="header">' + caption.header + '</div>';
						ret += '<div class="body">' + caption.body + '</div>';
						if (caption.footer)
							ret += '<div class="footer">' + caption.footer + '</div>';

						if (caption.table[0][0] != "") {
							ret += '<table class="table">'
							for (i in caption.table) {
								ret += '<tr>';
								for (j in caption.table[i]) {
									ret += '<td>' + caption.table[i][j] + '</td>';
								}
								ret += '</tr>'
							}
							ret += '</table>'
						}

						ret += '</div>';

						return ret;
					}(caption));
				}
			}
		}
	},

	hideTooltip: function() {
		this.jqueryElement.tooltip("open");
	},

	// </editor-fold>

	// <editor-fold> Localization
	localeData: {},
	loadLocaleData: function() {
		this.localeData = Multiload._jsonCommand('localization');
		if (this.localeData)
			console.log ("Loaded localization");
		else
			console.error ("Cannot load localization");
	},
	localizeText: function(string) {
		if (!this.localeData) {
			console.error ("Missing localization data");
			return string;
		}

		if (this.localeData[string]) {
			return this.localeData[string];
		} else {
			console.warn ("Missing localization for: '"+string+"'");
			return string;
		}
	},

	// </editor-fold>

	// <editor-fold> Persistence
	saveToFile: function() {
		return Multiload._jsonCommand('save').value;
	},
	reloadFromFile: function() {
		return Multiload._jsonCommand('reload').value;
	},
	// </editor-fold>

};


$(document).ready (function() { Multiload.init() });
