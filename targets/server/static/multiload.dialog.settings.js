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

	// <editor-fold> DOM injection
	var container = $('#settings-dialog');

	container.append(
		Multiload.localizeText('Drag and drop to arrange elements, click an element to configure it')+
		'<br />'+
		'<ul id="settingsElements"></ul>'+
		'<div id="settingsElementConfig">TEST</div>'
	);
	// </editor-fold>

	updateList = function() {
		$('#settings-dialog #settingsElements').html('');
		for (i in Multiload.data.elements) {
			var item = Multiload.data.elements[i];

			var element = null;
			if (item.graph) {
				element = $('<li class="graph">')
					.append('<span class="ui-icon ui-icon-star"></span>')
					.append(Multiload.graphTypes[item.graph.type].label)
					.append('<button class="delete ml-button"></button>')
					.append('<button class="pause ml-button"></button>')
					.append('<button class="play ml-button"></button>');
			} else {
				element = $('<li class="separator">')
					.append('<span class="ui-icon ui-icon-caret-2-n-s"></span>')
					.append('(' + Multiload.localizeText('separator') + ')')
					.append('<button class="delete ml-button"></button>');
			}

			element.click(function() {
				configureElement($(this).index());
			});

			$('#settingsElements').append(element);
		}

		$('#settings-dialog #settingsElements li button.delete.ml-button')
		.click(function() {
			var item = $(this).closest('li');
			if (Multiload.deleteElement(item.index()))
				item.remove();
		})
		.button({
			icon: "ui-icon-trash",
		});

		$('#settings-dialog #settingsElements li button.play.ml-button')
		.click(function() {
			var item = $(this).closest('li');
			Multiload.resumeGraph(item.index());
		})
		.button({
			icon: "ui-icon-play",
		});

		$('#settings-dialog #settingsElements li button.pause.ml-button')
		.click(function() {
			var item = $(this).closest('li');
			Multiload.pauseGraph(item.index());
		})
		.button({
			icon: "ui-icon-pause",
		});
	}

	configureElement = function(index) {
		if (!Multiload.data || !Multiload.data.elements)
			return;

		var element = Multiload.data.elements[index];

		var j = $('#settingsElementConfig');
		j.html("");

		j.append('<input type="hidden" id="settingsElementConfigIndex" value="' + index + '" />');
		j.append('<input type="hidden" id="settingsElementConfigType" value="' + element.type + '" />');

		j.append(MultiloadUi.makeNumericControl({
			id:       'settingsElementConfigSize',
			label:    Multiload.localizeText('Size'),
			value:    element.size,
			min:      Multiload.constants.elementSize.min,
			max:      Multiload.constants.elementSize.max,
			step:     1,
			onSpin:   function(event, ui) {
				if ($('#settingsElementConfigType').val() != "graph")
					return true;

				var borderSize = $('#settingsElementConfigBorder').val();
				if (ui.value < 1 + 2*borderSize)
					return false;
			},
			onChange: function(event, ui) {
				var index = $('#settingsElementConfigIndex').val();
				var value = $('#settingsElementConfigSize').val();
				Multiload.setElementSize(index, value);
			}
		}));

		if (element.type == "graph" && element.graph) {
			j.append(MultiloadUi.makeNumericControl({
				id:       'settingsElementConfigBorder',
				label:    Multiload.localizeText('Border size'),
				value:    element.graph.border,
				min:      Multiload.constants.graphBorder.min,
				max:      Multiload.constants.graphBorder.max,
				step:     1,
				onSpin:   function(event, ui) {
					var graphSize = $('#settingsElementConfigSize').val();
					if (ui.value > (graphSize)/2 - 1)
						return false;
				},
				onChange: function(event, ui) {
					var index = $('#settingsElementConfigIndex').val();
					var value = $('#settingsElementConfigBorder').val();
					Multiload.setGraphBorder(index, value);
				}
			}));

			j.append(MultiloadUi.makeNumericControl({
				id:       'settingsElementConfigCeiling',
				label:    Multiload.localizeText('Ceiling'),
				value:    element.graph.ceiling,
				min:      Multiload.constants.graphCeiling.min,
				max:      Multiload.constants.graphCeiling.max,
				step:     1,
				onChange: function(event, ui) {
					var index = $('#settingsElementConfigIndex').val();
					var value = $('#settingsElementConfigCeiling').val();
					Multiload.setGraphCeiling(index, value);
				}
			}));

			j.append(MultiloadUi.makeNumericControl({
				id:       'settingsElementConfigInterval',
				label:    Multiload.localizeText('Update interval (milliseconds)'),
				value:    element.interval,
				min:      Multiload.constants.graphInterval.min,
				max:      Multiload.constants.graphInterval.max,
				step:     10,
				onChange: function(event, ui) {
					var index = $('#settingsElementConfigIndex').val();
					var value = $('#settingsElementConfigInterval').val();
					Multiload.setGraphInterval(index, value);
				}
			}));

			var config = Multiload.getConfigEntries(index);
			if (config) {
				j.append('<label for="settingsElementConfigEntries">' + Multiload.localizeText('Graph configuration') + '</label>');
				var entriesControl = $('<div id="settingsElementConfigEntries"></div>');
				j.append(entriesControl);

				for (i in config) { // TODO controls here
					entriesControl.append('<div>'+config[i].label+' = '+Multiload.getGraphConfig(index, i).value+'</div>');
				}
			}
		}
	}

	$('#settings-dialog #settingsElements').sortable({
		axis: "y",
		containment: "parent",
		cursor: "move",
		opacity: 0.5,
		start: function(event, ui) {
			$('#settingsElements').data("ml-drag-start-position", ui.item.index());
		},
		stop: function(event, ui) {
			var startPosition = $('#settingsElements').data("ml-drag-start-position");
			var endPosition = ui.item.index();
			if (!Multiload.moveElement(startPosition, endPosition))
				updateList();
		},

	});
	$('#settings-dialog #settingsElements').disableSelection();

	$('#settings-dialog').dialog({
		autoOpen: false,
		buttons: [
			{
				text: Multiload.localizeText('Ok'),
				icons: {
					primary: "ui-icon-heart"
				},
				click: function() {
					$(this).dialog("close");
				}
			}
		],
		closeOnEscape: true,
		draggable: true,
		resizable: true,
		show: false,
		hide: false,
		modal: true,
		position: { my: "top", at: "bottom+10", of: ".headerbar" },
		width: 700,
		height: 580,
		title: Multiload.localizeText('Settings'),
		open: function(event, ui) {
			updateList();
		}
	});


});
