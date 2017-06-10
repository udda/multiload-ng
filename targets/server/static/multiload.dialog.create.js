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


$(document).ready(function() {

	var getSelectedElementType = function () {
		var items = $('#create-dialog input[name=element-type]');

		if (items[0].checked)
			return items[0].value;

		if (items[1].checked)
			return items[1].value;

		return null;
	}

	var getSelectedGraphType = function () {
		return $('#createGraphType').val();
	}

	var updateButtonStatus = function() {
		var btn = $('#createButtonCreate').button('widget');

		if (getSelectedElementType() == "separator") { // separator
			btn.show();
		} else { // graph
			if (getSelectedGraphType() !== null)
				btn.show();
			else
				btn.hide();
		}
	}


	var container = $('#create-dialog');

	// Element type
	container.append(
		'<label for="createElementTypeGroup">'     + Multiload.localizeText('Element type')   + '</label>'
	);
	var radioCreateGroup = $('<div id="createElementTypeGroup">').html (
		'<label for="createElementTypeGraph">'     + Multiload.localizeText('Graph')          + '</label>' +
		'<input type="radio" name="element-type" id="createElementTypeGraph" value="graph" checked="checked">' +
		'<label for="createElementTypeSeparator">' + Multiload.localizeText('Separator')      + '</label>' +
		'<input type="radio" name="element-type" id="createElementTypeSeparator" value="separator">'
	);
	radioCreateGroup.controlgroup();
	radioCreateGroup.find('input').change(function() {
		updateButtonStatus ();

		if (getSelectedElementType() == "separator")
			$('#createGraphContainer').hide();
		else
			$('#createGraphContainer').show();
	})
	.checkboxradio();
	container.append (radioCreateGroup);

	// Size
	container.append('<label for="createElementSize">' + Multiload.localizeText('Size') + '</label>');
	var createElementSize = $('<input id="createElementSize">').val(Multiload.constants.elementSize.default);
	container.append(createElementSize);
	createElementSize.spinner({
		min: Multiload.constants.elementSize.min,
		max: Multiload.constants.elementSize.max,
		spin: function(event, ui) {
			if (getSelectedElementType() != "graph")
				return;

			var borderSize = $('#createGraphBorder').val();
			if (ui.value < 1 + 2*borderSize)
				return false;
		}
	});

	// Graph settings container
	var createGraphContainer = $('<div id="createGraphContainer">');
	container.append(createGraphContainer);

	// Graph type
	createGraphContainer.append('<label for="createGraphType">' + Multiload.localizeText('Graph type') + '</label>');
	var createGraphType = $('<select id="createGraphType">').html(
		'<option disabled="disabled" selected="selected">' + Multiload.localizeText('Please pick one') + '</option>'
	);
	createGraphContainer.append (createGraphType);
	for (item in Multiload.graphTypes) {
		createGraphType.append($('<option>', { 
			value: Multiload.graphTypes[item].name,
			text : Multiload.graphTypes[item].label 
		}));
	}
	createGraphType.selectmenu({
		change: function(event, ui) {
			updateButtonStatus ();

			var type = Multiload.graphTypes[ui.item.value];
			if (type == undefined)
				return;

			$('#createGraphTypeDetails .description').html(type.description);
			$('#createGraphTypeDetails .helptext').html(type.helptext);
		}
	});

	// Graph type details
	createGraphContainer.append(
		'<div id="createGraphTypeDetails">' +
			'<div class="description">' + Multiload.localizeText('Please select a graph type') + '</div>' +
			'<div class="helptext">' + Multiload.localizeText('This section will display additional info.') + '</div>' +
		'</div>'
	);

	// Border size
	createGraphContainer.append('<label for="createGraphBorder">' + Multiload.localizeText('Border size') + '</label>');
	var createGraphBorder = $('<input id="createGraphBorder">').val(Multiload.constants.graphBorder.default);
	createGraphContainer.append(createGraphBorder);
	createGraphBorder.spinner({
		min: Multiload.constants.graphBorder.min,
		max: Multiload.constants.graphBorder.max,
		spin: function(event, ui) {
			if (getSelectedElementType() != "graph")
				return;

			var graphSize = $('#createElementSize').val();
			if (ui.value > (graphSize)/2 - 1)
				return false;
		}
	});

	// Update interval
	createGraphContainer.append('<label for="createGraphInterval">' + Multiload.localizeText('Update interval (milliseconds)') + '</label>');
	var createGraphInterval = $('<input id="createGraphInterval">').val(Multiload.constants.graphInterval.default);
	createGraphContainer.append(createGraphInterval);
	createGraphInterval.spinner({
		min: Multiload.constants.graphInterval.min,
		max: Multiload.constants.graphInterval.max,
		step: 10,
	});


	// Dialog
	container.dialog({
		autoOpen: false,
		buttons: [
			{
				id: "createButtonCreate",
				text: Multiload.localizeText('Create'),
				icons: {
					primary: "ui-icon-plus"
				},
				click: function() {
					var size = $('#createElementSize').val();

					var ret = {};

					if ($('#createElementTypeGraph')[0].checked) {
						var type = $('#createGraphType').val();
						var border = $('#createGraphBorder').val();
						var interval = $('#createGraphInterval').val();

						if (type == null) {
							alert(Multiload.localizeText('Please select graph type!'));
							return;
						}

						ret = Multiload.createGraph(size, type, border, interval, -1);
					} else {
						ret = Multiload.createSeparator(size, -1);
					}

					if (ret === undefined || ret === null || ret < 0) {
						alert(Multiload.localizeText('Cannot create element!'));
					} else {
						console.log ("Created element at #" + ret);
					}

					$(this).dialog("close");
				}
			},
			{
				id: "createButtonCancel",
				text: Multiload.localizeText('Cancel'),
				icons: {
					primary: "ui-icon-cancel"
				},
				click: function() {
					$(this).dialog("close");
				}
			}
		],
		closeOnEscape: true,
		draggable: true,
		resizable: false,
		show: false,
		hide: false,
		modal: true,
		width: 500,
		position: { my: "top", at: "bottom+10", of: ".headerbar" },
		title: Multiload.localizeText('Create element'),
		create: function(event, ui) {
			updateButtonStatus();
		},
	});

});
