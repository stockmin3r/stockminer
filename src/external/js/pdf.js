/*The MIT License (MIT)
 *
 *Copyright (c) 2017 [Muaz Khan](https://github.com/muaz-khan)
 *
 *Permission is hereby granted, free of charge, to any person obtaining a copy of
 *this software and associated documentation files (the "Software"), to deal in
 *the Software without restriction, including without limitation the rights to
 *use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 *the Software, and to permit persons to whom the Software is furnished to do so,
 *subject to the following conditions:
 *
 *The above copyright notice and this permission notice shall be included in all
 *copies or substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 *COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 * https://github.com/muaz-khan/Canvas-Designer
 */

/* From WebRTC-Project with adaptations for usage in Workspaces
 * The purpose is to be able to draw on differnet pages of a PDF,
 * create "notes" or "shortcuts" with explanations of certain parts
 * of a PDF page, and to be able to "squeak" a drawed-on-PDF
 */
((window, factory) => {window.CanvasDesigner = factory(window, window.Draggabilly)})(window, (window, Draggabilly) => {
	var icons = {};
	var data_uris = {
		pdf_next:  'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAYAAADDPmHLAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAAEnQAABJ0Ad5mH3gAAAAYdEVYdFNvZnR3YXJlAHBhaW50Lm5ldCA0LjEuMWMqnEsAAAYASURBVHhe7d3RjRs3EMZxPwRwAwFcQAA3EMANBK4gcAOBKwjcQOAKzh3EFcQdxB1cCdeBS1DyKVhYof/y6cghOeTMw+/l81niLme5XO5KenY6nVJgGKY4MExxYJjiwDDFgWGKA8MUB4YpDgxTHBimODBMcWCY4sAwxYFhigPDFAeGKQ4MUxwYpjgwTHFguIuHh4fT358/n/54//7sl9evH/Xrmzfnv7378OH8f798+fLvS/Hr7wDDVd3f3587Tp3444sXpx+ePzfx08uXp9/evj39+fHjSUVVvu/KMFyJOv33d+/OnUSd18PPr16dC22HYsDQOw3L6oCRnX6NThsaGco2rgJDr3TE6fxsObxbUTGqbavNGTD0RjvVa8eX1EaNTuU2eIWhJ9qZK3R8SSOCriLK7fEGQw80udP5lXbuSnRF4vm0gOFsqx7112hbvI4GGM6iI0VHDO3EHehytdzm2TCcQUO+rq9px+1EpzVPpwQMR1Pn7zTkP0aF7mURCcOR/vr0KVTnH7TNKvxyf4yG4ShaQaOdE4WHIsBwBG047ZRoZhcBhr1FO+c/RnOCWRNDDHvShmbnf0tFUO6rETDsKcKlXi09c1Dur94w7EULIbTh6avRt5Yx7EFLobTB6f90ehy5RoChNZ33PTy8sQqtFpb7sBcMreXQ/3SjTgUYWlrlel9Dr6erE7VlxKUhhpZWuKevna1C9bY+MeLuIYZWVpj4HZ1/tNlbEfSeEGJoxfvRX3b+wVMR9F4bwNCC96P/WucfPBVBz1EAQwuej/7HOv/gpQj0RHTZNisYtlLF0oZ4cGvnHzwUgd6/bJcVDFt5ve5/aucfPBSBHpwp22UBw1YeV/1qO188PLiih2XLdlnAsIV2Mm3ATKt3/qHHwhCGLbwN/7t0vvQ4DWDYwtP9/p06X3qsCWBYS0MUNXyG3TpfNLcq29oKw1peFn927PyD9TwAw1pasKBGj7Rz54v1ZwwxrDX7c327d75YrwpiWGvmBDBC54v1egCGtajBI0TpfLF+XAzDGrOuACJ1vmh7y+1ogWGNGVcA0Tr/UG5LCwxrjC6AqJ0v5fa0wLDGyAKI3PliuRaAYY1RBRC988VyLQDDGqMKoPY6WEeNiodeczWhC0B0JJfvfwuNHDsUQfgCkMhFUG5TCwxrjC4AiVoE5fa0wLDGjAKQiEVQbksLDGtRY0eIVATWzwRgWIsaPEqUInB7L0BmfxgkQhFYPxaGYS01jho90u5FcGf8WwQY1vLwRJDsXASWawCCYa1ZVwKktgg8bQMp29sKwxbU6Flqi0D/j15vth7fJYhhC2+fCt6pCHp8YwiGLTRJocbPtEsRWJ//BcMWXj8avnoRaHJats0Chq28fh3sykXQ66tiMGzl8TRwqCkCD5eHakPZLgsYtvL0GUHylCLw0Pk9PhN4wNCCh1XB77mlCLwsDNWeum6BoQXP3xN0+N6O1b956Hy1wfoDoZcwtLLCbwBSESijv52h5zeECYZWNITSRnlzWQSeOr/30S8YWvI+Fzio4z11vtwZ3/kjGFrSXMDDuXQ1PWf+lzC0pkqmjUzX9Vj2JRj2kD8WdbsRXxN/wLAHL9fU3mno7z3xu4RhL94mWR71WvK9BsOeVrkqmOHycnQUDHvL+cC3et3tewyGvekcl0XwlfWz/k+B4Qg5KfyPDoSRk74ShqNEL4LZnS8YjqQi0KUP7aCdeeh8wXC0aHMCTfg8dL5gOIN2yAq3j1v1vr37VBjOtOt9A811ev3uTwsMZ9ttXqDLvJ6//dcCQw90SvD662O30lE/4p5+Cww90Wjg7eNmt9BEz+tRfwlDj3T+XOG0oGIddS/fAoae6YaJxxFBVzArdfwBwxXo1KBhduZKokYkzVNWGOqvwXA1Oj2MKgZ1ut7L4yVdDQxXppFBM28NyRZzBq1QqsP1mnrt8v1Wh+FudG4WrcIdNHRrLiGXuehvd+xsgmGKA8MUB4YpDgxTHBimODBMcWCY4sAwxYFhigPDFAeGKQ4MUxwYpjgwTHFgmOLAMMWBYYoDwxTF6dk/P8ngRg7z9FEAAAAASUVORK5CYII=',
		pdf_prev:  'data:image/png;base64,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',
		pdf_close: 'data:image/png;base64,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'
	};

	const forLoop = `
		for(i; i < length; i++) {
			p = points[i];
			point = p[1];
			context.beginPath();
			if(p[2]) { 
				context.lineWidth = p[2][0];
				context.strokeStyle = p[2][1];
				context.fillStyle = p[2][2];
				context.globalAlpha = p[2][3];
				context.globalCompositeOperation = p[2][4];
				context.lineCap = p[2][5];
				context.lineJoin = p[2][6];
				context.font = p[2][7];
			}
			if(p[0] === "line") { 
				context.moveTo(point[0], point[1]);
				context.lineTo(point[2], point[3]);
			}
			if(p[0] === "arrow")
				drawArrow(self, point[0], point[1], point[2], point[3], p[2]);
			if(p[0] === "pencil") {
				context.moveTo(point[0], point[1]);
				context.lineTo(point[2], point[3]);
			}
			if(p[0] === "marker") {
				context.moveTo(point[0], point[1]);
				context.lineTo(point[2], point[3]);
			}
			if(p[0] === "text")
				context.fillText(point[0], point[1], point[2]);
			if(p[0] === "eraser") {
				context.moveTo(point[0], point[1]);
				context.lineTo(point[2], point[3]);
			}
			if(p[0] === "arc")
				context.arc(point[0], point[1], point[2], point[3], 0, point[4]); 
			if(p[0] === "rect") {
				context.strokeRect(point[0], point[1], point[2], point[3]);
				context.fillRect(point[0], point[1], point[2], point[3]);
			}
			if(p[0] === "quadratic") {
				context.moveTo(point[0], point[1]);
				context.quadraticCurveTo(point[2], point[3], point[4], point[5]);
			}
			if(p[0] === "bezier") {
				context.moveTo(point[0], point[1]);
				context.bezierCurveTo(point[2], point[3], point[4], point[5], point[6], point[7]);
			}
			context.stroke();
			context.fill();`;

	const strokeFillText = `
		function strokeOrFill(lineWidth, strokeStyle, fillStyle, globalAlpha, globalCompositeOperation, lineCap, lineJoin, font) {
			if(lineWidth) {
				context.globalAlpha = globalAlpha;
				context.globalCompositeOperation = globalCompositeOperation;
				context.lineCap = lineCap;
				context.lineJoin = lineJoin;
				context.lineWidth = lineWidth;
				context.strokeStyle = strokeStyle;
				context.fillStyle = fillStyle;
				context.font = font;
			}
			context.stroke();
			context.fill();`;


	var N = 0; // Number Of CanvasDesigner Instances

	class CanvasDesigner {
		/* ************
		 * CLASS INIT *
		 *************/
		constructor(obj) {
			this.GID                  = N++;  // instance ID of this object
			var self                  = this, GID = this.GID;
			self.width                = obj.parentNode.clientWidth;
			self.height               = obj.parentNode.clientHeight;
			this.points               = [];
			this.obj                  = obj;
			this.context              = CTX(self,'main-canvas');
			this.tempContext          = CTX(self,'temp-canvas');
			this.tool                 = 'pencil';
			this.cache                = {};
			this.isTouch              = 'createTouch' in document;
			this.canvas               = this.tempContext.canvas;
			this.pencilLineWidth      = ID(obj,'pencil-stroke-style').value;
			this.pencilStrokeStyle    = ID(obj,'pencil-fill-style').value;
			this.fillStyle            = 'rgba(0,0,0,0)';
			this.globalAlpha          = 1;
			this.globalCompositeOperation = 'source-over';
			this.lineCap              = 'round';
			this.font                 = '15px "Arial"';
			this.lineJoin             = 'round';
			this.lineWidth            = 2,
			this.strokeStyle          = '#6c96c8',
			this.copiedStuff          = [];
			this.isControlKeyPressed  = 0;
			this.textarea             = ID(obj,'code-text');
			this.codeText             = ID(obj,'code-text');
			this.optionsContainer     = ID(obj,'options-container');
			this.isAbsolute           = ID(obj,'is-absolute-points');
			this.isShorten            = ID(obj,'is-shorten-code');
			this.designPreview        = ID(obj,'design-preview');
			this.codePreview          = ID(obj,'code-preview');
			this.lineCapSelect        = ID(obj,'lineCap-select');
			this.lineJoinSelect       = ID(obj,'lineJoin-select');
   		 	this.toolBox              = ID(obj,'tool-box');
			this.toolBox.style.height = (innerHeight) + 'px'; // -toolBox.offsetTop - 77
			this.lastPointIndex       = 0;
			this.uid                  = 0;
			this.keyCode              = 0;
			this.icons                = {};
			this.markerLineWidth      = ID(obj,'marker-stroke-style');
			this.markerStrokeStyle    = '#' + ID(obj,'marker-fill-style');
			this.markerGlobalAlpha    = 0.7;
			this.pdfHandler           = pdfHandler;
			this.draw                 = drawHelper;

			addEvent(document, 'keydown',  function(e){onkeydown(self,e)});
			addEvent(document, 'keyup',    function(e){onkeyup(self,e)});
			addEvent(document, 'keypress', function(e){onkeypress(self,e)});
			addEvent(document, 'paste',    function(e){onTextFromClipboard(self,e)});

			this.pdf = {
				ismousedown:false,
				prevX:0,
				prevY:0,
				lastPdfURL:null,
				lastIndex:0,
				lastPointIndex:0,
				removeWhiteBackground:false,
				pdfPageContainer:ID(obj,'pdf-page-container'),
				pdfPagesList:    ID(obj,'pdf-pages-list'),
				pdfNext:         ID(obj,'pdf-next'),
				pdfPrev:         ID(obj,'pdf-prev'),
				pdfClose:        ID(obj,'pdf-close'),
				pageNumber:1,
				images:[],
			};
			this.bezier = {
				ismousedown:false,
				prevX:0,
				prevY:0,
				firstControlPointX:0,
				firstControlPointY:0,
				secondControlPointX:0,
				secondControlPointY:0,
				isFirstStep:true,
				isSecondStep:false,
				isLastStep:false
			};
			this.zoom   = {scale:1.0};
			this.pencil = {ismousedown:false,prevX:0,prevY:0};
			this.eraser = {ismousedown:false,prevX:0,prevY:0};
			this.line   = {ismousedown:false,prevX:0,prevY:0};
			this.rect   = {ismousedown:false,prevX:0,prevY:0};
			this.marker = {ismousedown:false,prevX:0,prevY:0};
			this.arrow  = {ismousedown:false,prevX:0,prevY:0,arrowSize:10}
			this.drag   = {ismousedown:false,prevX:0,prevY:0,pointsToMove:'all',startingIndex:0};
			this.image  = {ismousedown:false,prevX:0,prevY:0,lastImageURL:null,lastImageIndex:0,images:[]};
			this.quad   = {ismousedown:false,prevX:0,prevY:0,controlPointX:0,controlPointY:0,isFirstStep:true,isLastStep:false};
			this.arc    = {ismousedown:false,prevX:0,prevY:0,prevRadius:0,isCircleDrawn:false,isCircledEnded:true,isClockwise:false,arcRangeContainer:null,arcRange:null}
			this.text   = {text:'',selectedFontFamily:'Arial',selectedFontSize:'15',lastFillStyle: '',blinkCursorInterval:null,index:0};

			arcHandler.init(self);
			addEvent(this.designPreview, 'click', function() {
				selectBtn(this.designPreview);
				btnDesignerPreviewClicked();
			});
			addEvent(this.codePreview, 'click', function() {
				selectBtn(this.codePreview);
				btnCodePreviewClicked();
			});
			addEvent(this.isShorten,  'change', function(){updateTextArea(self)});
			addEvent(this.isAbsolute, 'change', function(){updateTextArea(self)});

			addEvent(this.canvas, this.isTouch ? 'touchstart mousedown' : 'mousedown', function(e) {
				if (self.isTouch) e = e.pageX ? e : e.touches.length ? e.touches[0] : {pageX:0,pageY:0};
				handlers[self.tool].mousedown(self,e);
				if (self.tool != 'pdf')
					drawHelper.redraw(self);
				preventStopEvent(e);
			});
			addEvent(this.canvas, this.isTouch ? 'touchend touchcancel mouseup' : 'mouseup', function(e) {
				if (self.isTouch && (!e || !('pageX' in e))) {
					if (e && e.touches && e.touches.length) {
						e = e.touches[0];
					} else if (e && e.changedTouches && e.changedTouches.length) {
						e = e.changedTouches[0];
					} else {
						e = {
							pageX:0,
							pageY:0
						}
					}
				}
				handlers[self.tool].mouseup(self,e);
				if (self.tool != 'pdf')
					drawHelper.redraw(self);
				preventStopEvent(e);
//				syncPoints(is.isDragAllPaths || is.isDragLastPath ? true : false);
				preventStopEvent(e);
			});
			addEvent(this.canvas, this.isTouch ? 'touchmove mousemove' : 'mousemove', function(e) {
				if (self.isTouch) e = e.pageX ? e : e.touches.length ? e.touches[0] : {pageX:0,pageY:0};
				handlers[self.tool].mousemove(self,e);
				if (self.tool != 'pdf')
					drawHelper.redraw(self);
				preventStopEvent(e);
			});
/*
			decorateDragLastPath();
			decorateDragAllPaths();
			ID('drag-last-path').style.display = 'block';
			ID('drag-all-paths').style.display = 'block';
			decorateLine();
			document.getElementById('line').style.display = 'block';
			decorateUndo();
			document.getElementById('undo').style.display = 'block';
			decorateArrow();
			document.getElementById('arrow').style.display = 'block';
			decoreZoomUp();
			decoreZoomDown();
			ID('zoom-up').style.display = 'block';
			ID('zoom-down').style.display = 'block';
			decorateColors();
			document.getElementById('colors').style.display = 'block';
			decorateAdditionalOptions();
			document.getElementById('additional').style.display = 'block';
			decoratePencil();
			document.getElementById('pencil-icon').style.display = 'block';
			decorateMarker();
			document.getElementById('marker-icon').style.display = 'block';
			decorateEraser();
			document.getElementById('eraser-icon').style.display = 'block';
			decorateText();
			document.getElementById('text-icon').style.display = 'block';
			decorateImage();
			document.getElementById('image-icon').style.display = 'block';
			decoratePDF();
			document.getElementById('pdf-icon').style.display = 'block';
			decorateArc();
			document.getElementById('arc').style.display = 'block';
			decorateRect();
			document.getElementById('rectangle').style.display = 'block';
			decorateQuadratic();
			document.getElementById('quadratic-curve').style.display = 'block';
			decorateBezier();
			document.getElementById('bezier-curve').style.display = 'block';*/

		} // constructor

		/* ***************
		 * CLASS OBJECTS *
		 ****************/
		FileSelector = function() {
			var selector = this;
			selector.selectSingleFile = selectFile;
			selector.selectMultipleFiles = function(callback) {
				selectFile(callback, true);
			};
			function selectFile(callback, multiple, accept) {
				var file = document.createElement('input');
				file.type = 'file';
				if (multiple)
					file.multiple = true;
				file.accept = accept || 'image/*';
				file.onchange = function() {
					if (multiple) {
						if (!file.files.length) {
							console.error('No file selected.');
							return;
						}
						callback(file.files);
						return;
					}
					if (!file.files[0]) {
						console.error('No file selected.');
						return;
					}
					callback(file.files[0]);
					file.parentNode.removeChild(file);
				};
				file.style.display = 'none';
				(document.body || document.documentElement).appendChild(file);
				fireClickEvent(file);
			}
			function fireClickEvent(element) {
				var evt = new window.MouseEvent('click', {
					view: window,
					bubbles: true,
					cancelable: true,
					button: 0,
					buttons: 0,
					mozInputSource: 1
				});
				var fired = element.dispatchEvent(evt);
			}
		};
		/* ***************
		 * CLASS METHODS *
		 ****************/
		setSelection(self,element,tool) {
			endLastPath(self);
//			hideContainers(self.GID);
			self.tool = tool;
	/*		var selected = document.getElementsByClassName('selected-shape')[0];
			if (selected) selected.className = selected.className.replace(/selected-shape/g, '');
			if (!element.className)
				element.className = '';
			element.className += ' selected-shape';*/
		}

		// ENDOF Class Methods
	} // ENDOF Class CanvasDesigner

	/* ****************
	 * GLOBAL METHODS *
	 *****************/
	function addEvent(element, eventType, callback) {
		if (eventType.split(' ').length > 1) {
			var events = eventType.split(' ');
			for (var i = 0; i < events.length; i++)
				addEvent(element, events[i], callback);
			return;
		}
		if (element.addEventListener) {
			element.addEventListener(eventType, callback, !1);
			return true;
		} else if (element.attachEvent) {
			return element.attachEvent('on' + eventType, callback);
		} else {
			element['on' + eventType] = callback;
		}
		return this;
	}
	function ID(obj,c) {return obj.querySelector("."+c)}
	function CTX(self,c) {
		var canv = ID(self.obj,c),
		ctx      = canv.getContext('2d');
		console.log("getContext: " + c);
		canv.setAttribute('width', self.width);
		canv.setAttribute('height', self.height);
		ctx.lineWidth   = self.lineWidth;
		ctx.strokeStyle = self.strokeStyle;
		ctx.fillStyle   = self.fillStyle;
		ctx.font		= self.font;
		return ctx;
	}
	function getContext(obj,c) {
		var context         = ID(obj,c).getContext('2d');
		context.lineWidth   = 2;
		context.strokeStyle = '#6c96c8';
		return context;
	}
	function hideContainers(obj) {
		ID(obj,'additional-container').style.display='none';
		ID(obj,'colors-container').style.display='none';
		ID(obj,'marker-container').style.display='none';
		ID(obj,'pencil-container').style.display='none';
		ID(obj,'line-width-container').style.display='none';
		ID(obj,'marker-fill-colors').style.display='none';
		ID(obj,'pencil-fill-colors').style.display='none';
	}
	function preventStopEvent(e) {
		if (!e)
			return;
		if (typeof e.preventDefault === 'function')
			e.preventDefault();
		if (typeof e.stopPropagation === 'function')
			e.stopPropagation();
	}
	function hexToR(h)      {return parseInt((cutHex(h)).substring(0, 2), 16)}
	function hexToG(h)      {return parseInt((cutHex(h)).substring(2, 4), 16)}
	function hexToB(h)      {return parseInt((cutHex(h)).substring(4, 6), 16)}
	function cutHex(h)      {return (h.charAt(0) == "#") ? h.substring(1, 7) : h}
	function hexToRGB(h)    {return [hexToR(h),hexToG(h),hexToB(h)]}
	function toFixed(input) {return Number(input).toFixed(1)}
	function clone(obj)     {
		if (obj === null || typeof(obj) !== 'object' || 'isActiveClone' in obj)
			return obj;
		if (obj instanceof Date)
			var temp = new obj.constructor(); //or new Date(obj);
		else
			var temp = obj.constructor();
		for (var key in obj) {
			if (Object.prototype.hasOwnProperty.call(obj, key)) {
				obj['isActiveClone'] = null;
				temp[key] = clone(obj[key]);
				delete obj['isActiveClone'];
			}
		}
		return temp;
	}
	function updateTextArea(self) {
		var isAbsolutePoints = ID(self.obj,'is-absolute-points').checked,
			isShortenCode    = ID(self.obj,'is-shorten-code').checked;	
		if (isAbsolutePoints  && isShortenCode)  absoluteShortened();
		if (isAbsolutePoints  && !isShortenCode) absoluteNOTShortened();
		if (!isAbsolutePoints && isShortenCode)  relativeShortened   ();
		if (!isAbsolutePoints && !isShortenCode) relativeNOTShortened();
	}
	function getPoint(pointToCompare, compareWith, prefix) {
		if (pointToCompare > compareWith)      pointToCompare = prefix + ' + ' + (pointToCompare - compareWith);
		else if (pointToCompare < compareWith) pointToCompare = prefix + ' - ' + (compareWith    - pointToCompare);
		else pointToCompare = prefix;
		return pointToCompare;
	}
	function absoluteShortened(self) {
		var output = '', points = self.points, length = points.length, i = 0,point;
		for (i; i < length; i++) {
			point = points[i];
			output += this.shortenHelper(self, point[0], point[1], point[2]);
		}
		output = output.substr(0, output.length - 2);
		textarea.value = 'var points = [' + output + '], length = points.length, point, p, i = 0;\n\n' + drawArrow.toString() + '\n\n' + forLoop;
		self.prevProps = null;
	}
	function absoluteNOTShortened(self) {
		var tempArray = [], points = self.points, length = points.length, point, p;
		for (var i = 0; i < length; i++) {
			p = points[i];
			point = p[1];
			if (p[0] === 'pencil')
				tempArray[i] = ['context.beginPath();\n' + 'context.moveTo(' + point[0] + ', ' + point[1] + ');\n' + 'context.lineTo(' + point[2] + ', ' + point[3] + ');\n' + strokeOrFill(self, p[2])];
			if (p[0] === 'marker')
				tempArray[i] = ['context.beginPath();\n' + 'context.moveTo(' + point[0] + ', ' + point[1] + ');\n' + 'context.lineTo(' + point[2] + ', ' + point[3] + ');\n' + strokeOrFill(self, p[2])];
			if (p[0] === 'eraser')
				tempArray[i] = ['context.beginPath();\n' + 'context.moveTo(' + point[0] + ', ' + point[1] + ');\n' + 'context.lineTo(' + point[2] + ', ' + point[3] + ');\n' + strokeOrFill(self, p[2])];
			if (p[0] === 'line')
				tempArray[i] = ['context.beginPath();\n' + 'context.moveTo(' + point[0] + ', ' + point[1] + ');\n' + 'context.lineTo(' + point[2] + ', ' + point[3] + ');\n' + strokeOrFill(self, p[2])];
			if (p[0] === 'pencil')
				tempArray[i] = ['context.beginPath();\n' + 'context.moveTo(' + point[0] + ', ' + point[1] + ');\n' + 'context.lineTo(' + point[2] + ', ' + point[3] + ');\n' + strokeOrFill(self, p[2])];
			if (p[0] === 'text')
				tempArray[i] = [strokeOrFill(self, p[2]) + '\ncontext.fillText('  + point[0] + ', ' + point[1] + ', ' + point[2] + ');'];
			if (p[0] === 'arrow')
				tempArray[i] = ['drawArrow(self,' + point[0] + ', ' + point[1] + ', ' + point[2] + ', ' + point[3] + ', \'' + p[2].join('\',\'') + '\');'];
			if (p[0] === 'arc')
				tempArray[i] = ['context.beginPath(); \n' + 'context.arc(' + toFixed(point[0]) + ',' + toFixed(point[1]) + ',' + toFixed(point[2]) + ',' + toFixed(point[3]) + ', 0,' + point[4] + '); \n' + strokeOrFill(self, p[2])];
			if (p[0] === 'rect') 
				tempArray[i] = [strokeOrFill(self, p[2]) + '\n' + 'context.strokeRect(' + point[0] + ', ' + point[1] + ',' + point[2] + ',' + point[3] + ');\n' + 'context.fillRect(' + point[0] + ', ' + point[1] + ',' + point[2] + ',' + point[3] + ');'];
			if (p[0] === 'quadratic') 
				tempArray[i] = ['context.beginPath();\n' + 'context.moveTo(' + point[0] + ', ' + point[1] + ');\n' + 'context.quadraticCurveTo(' + point[2] + ', ' + point[3] + ', ' + point[4] + ', ' + point[5] + ');\n' + strokeOrFill(self, p[2])];
			if (p[0] === 'bezier')
				tempArray[i] = ['context.beginPath();\n'+'context.moveTo('+point[0]+', '+point[1]+');\n'+'context.bezierCurveTo('+point[2]+', '+point[3]+', '+point[4]+', '+point[5]+', '+point[6]+', '+point[7]+');\n'+strokeOrFill(self, p[2])];
		}
		textarea.value = tempArray.join('\n\n') + this.strokeFillText + '\n\n' + drawArrow.toString();
		self.prevProps = null;
	}
	function relativeShortened(self) {
		var i = 0, point, p, points = self.points, length = points.length, output = '', x = 0, y = 0;
		for (i; i < length; i++) {
			p = points[i];
			point = p[1];	
			if (i === 0) {
				x = point[0];
				y = point[1];
			}
			if (p[0] === 'text') {
				x = point[1];
				y = point[2];
			}
			if (p[0] === 'pencil')
				output += shortenHelper(p[0], [getPoint(point[0], x, 'x'),getPoint(point[1], y, 'y'),getPoint(point[2], x, 'x'),getPoint(point[3], y, 'y')], p[2]);
			if (p[0] === 'marker')
				output += shortenHelper(p[0], [getPoint(point[0], x, 'x'),getPoint(point[1], y, 'y'),getPoint(point[2], x, 'x'),getPoint(point[3], y, 'y')], p[2]);
			if (p[0] === 'eraser')
				output += shortenHelper(p[0], [getPoint(point[0], x, 'x'),getPoint(point[1], y, 'y'),getPoint(point[2], x, 'x'),getPoint(point[3], y, 'y')], p[2]);
			if (p[0] === 'line')
				output += shortenHelper(p[0], [getPoint(point[0], x, 'x'),getPoint(point[1], y, 'y'),getPoint(point[2], x, 'x'),getPoint(point[3], y, 'y')], p[2]);
			if (p[0] === 'pencil')
				output += shortenHelper(p[0], [getPoint(point[0], x, 'x'),getPoint(point[1], y, 'y'),getPoint(point[2], x, 'x'),getPoint(point[3], y, 'y')], p[2]);
			if (p[0] === 'arrow')
				output += shortenHelper(p[0], [getPoint(point[0], x, 'x'),getPoint(point[1], y, 'y'),getPoint(point[2], x, 'x'),getPoint(point[3], y, 'y')], p[2]);
			if (p[0] === 'text')
				output += shortenHelper(p[0], [point[0],getPoint(point[1], x, 'x'),getPoint(point[2], y, 'y')], p[2]);
			if (p[0] === 'arc')
				output += shortenHelper(p[0], [getPoint(point[0], x, 'x'),getPoint(point[1], y, 'y'),point[2],point[3],point[4]], p[2]);
			if (p[0] === 'rect')
				output += shortenHelper(p[0], [getPoint(point[0], x, 'x'),getPoint(point[1], y, 'y'),getPoint(point[2], x, 'x'),getPoint(point[3], y, 'y')], p[2]);
			if (p[0] === 'quadratic')
				output += shortenHelper(p[0], [getPoint(point[0], x, 'x'),getPoint(point[1], y, 'y'),getPoint(point[2], x, 'x'),getPoint(point[3], y, 'y'),getPoint(point[4], x, 'x'),getPoint(point[5], y, 'y')], p[2]);
			if (p[0] === 'bezier')
				output += shortenHelper(p[0], [getPoint(point[0], x, 'x'),getPoint(point[1], y, 'y'),getPoint(point[2], x, 'x'),getPoint(point[3], y, 'y'),getPoint(point[4], x, 'x'),getPoint(point[5], y, 'y'),getPoint(point[6], x, 'x'),getPoint(point[7], y, 'y')], p[2]);
		}
		output = output.substr(0, output.length - 2);
		textarea.value = 'var x = ' + x + ', y = ' + y + ', points = [' + output + '], length = points.length, point, p, i = 0;\n\n' + drawArrow.toString() + '\n\n' + this.forLoop;
		self.prevProps = null;
	}
	function relativeNOTShortened(self) {
		var i, point, p, points = self.points, length = points.length, output = '', x = 0, y = 0;
		for (i = 0; i < length; i++) {
			p = points[i];
			point = p[1];
			if (i === 0) {
				x = point[0];
				y = point[1];
				if (p[0] === 'text') {
					x = point[1];
					y = point[2];
				}
				output = 'var x = ' + x + ', y = ' + y + ';\n\n';
			}
			if (p[0] === 'arc')
				output += 'context.beginPath();\n' + 'context.arc(' + getPoint(point[0], x, 'x') + ', ' + getPoint(point[1], y, 'y') + ', ' + point[2] + ', ' + point[3] + ', 0, ' + point[4] + ');\n' + this.strokeOrFill(p[2]);
			if (p[0] === 'pencil')
				output += 'context.beginPath();\n' + 'context.moveTo(' + getPoint(point[0], x, 'x') + ', ' + getPoint(point[1], y, 'y') + ');\n' + 'context.lineTo(' + getPoint(point[2], x, 'x') + ', ' + getPoint(point[3], y, 'y') + ');\n' + this.strokeOrFill(p[2]);
			if (p[0] === 'marker')
				output += 'context.beginPath();\n' + 'context.moveTo(' + getPoint(point[0], x, 'x') + ', ' + getPoint(point[1], y, 'y') + ');\n' + 'context.lineTo(' + getPoint(point[2], x, 'x') + ', ' + getPoint(point[3], y, 'y') + ');\n' + this.strokeOrFill(p[2]);
			if (p[0] === 'eraser')
				output += 'context.beginPath();\n' + 'context.moveTo(' + getPoint(point[0], x, 'x') + ', ' + getPoint(point[1], y, 'y') + ');\n' + 'context.lineTo(' + getPoint(point[2], x, 'x') + ', ' + getPoint(point[3], y, 'y') + ');\n' + this.strokeOrFill(p[2]);
			if (p[0] === 'line')
				output += 'context.beginPath();\n' + 'context.moveTo(' + getPoint(point[0], x, 'x') + ', ' + getPoint(point[1], y, 'y') + ');\n' + 'context.lineTo(' + getPoint(point[2], x, 'x') + ', ' + getPoint(point[3], y, 'y') + ');\n' + this.strokeOrFill(p[2]);
			if (p[0] === 'pencil')
				output += 'context.beginPath();\n' + 'context.moveTo(' + getPoint(point[0], x, 'x') + ', ' + getPoint(point[1], y, 'y') + ');\n' + 'context.lineTo(' + getPoint(point[2], x, 'x') + ', ' + getPoint(point[3], y, 'y') + ');\n' + this.strokeOrFill(p[2]);
			if (p[0] === 'arrow')
				output += 'drawArrow(self,' + getPoint(point[0], x, 'x') + ', ' + getPoint(point[1], y, 'y') + ', ' + getPoint(point[2], x, 'x') + ', ' + getPoint(point[3], y, 'y') + ', \'' + p[2].join('\',\'') + '\');\n';
			if (p[0] === 'text')
				output += this.strokeOrFill(p[2]) + '\n' + 'context.fillText(' + point[0] + ', ' + getPoint(point[1], x, 'x') + ', ' + getPoint(point[2], y, 'y') + ');';
			if (p[0] === 'rect')
				output += this.strokeOrFill(p[2]) + '\n' + 'context.strokeRect(' + getPoint(point[0], x, 'x') + ', ' + getPoint(point[1], y, 'y') + ', ' + getPoint(point[2], x, 'x') + ', ' + getPoint(point[3], y, 'y') + ');\n' + 'context.fillRect(' + getPoint(point[0], x, 'x') + ', ' + getPoint(point[1], y, 'y') + ', ' + getPoint(point[2], x, 'x') + ', ' + getPoint(point[3], y, 'y') + ');';
			if (p[0] === 'quadratic')
				output += 'context.beginPath();\n' + 'context.moveTo(' + getPoint(point[0], x, 'x') + ', ' + getPoint(point[1], y, 'y') + ');\n' + 'context.quadraticCurveTo(' + getPoint(point[2], x, 'x') + ', ' + getPoint(point[3], y, 'y') + ', '   + getPoint(point[4], x, 'x') + ', ' + getPoint(point[5], y, 'y') + ');\n' + this.strokeOrFill(p[2]);
			if (p[0] === 'bezier')
				output += 'context.beginPath();\n' + 'context.moveTo(' + getPoint(point[0], x, 'x') + ', ' + getPoint(point[1], y, 'y') + ');\n' + 'context.bezierCurveTo('	+ getPoint(point[2], x, 'x') + ', ' + getPoint(point[3], y, 'y') + ', '   + getPoint(point[4], x, 'x') + ', ' + getPoint(point[5], y, 'y') + ', ' + getPoint(point[6], x, 'x') + ', ' + getPoint(point[7], y, 'y') + ');\n' + this.strokeOrFill(p[2]);
			if (i !== length - 1) output += '\n\n';
		}
		textarea.value = output + this.strokeFillText + '\n\n' + drawArrow.toString();
		self.prevProps = null;
	}
	function strokeOrFill(self,p) {
		if (!self.prevProps || self.prevProps !== p.join(',')) {
			self.prevProps = p.join(',');
			return 'strokeOrFill(\'' + p.join('\', \'') + '\');';
		}
		return 'strokeOrFill();';
	}
	function shortenHelper(self, name, p1, p2) {
		var result = '[\'' + name + '\', [' + p1.join(', ') + ']';
		if (!self.prevProps || self.prevProps !== p2.join(',')) {
			self.prevProps = p2.join(',');
			result += ', [\'' + p2.join('\', \'') + '\']';
		}
		return result + '], ';
	}
	function drawArrow(self, mx, my, lx, ly, options) {
		function getOptions(self, opt) {
			opt = opt || {};
			return [
				opt.lineWidth   || 2,
				opt.strokeStyle || '#6c96c8',
				opt.fillStyle   || 'rgba(0,0,0,0)',
				opt.globalAlpha || 1,
				opt.globalCompositeOperation || 'source-over',
				opt.lineCap     || 'round',
				opt.lineJoin    || 'round',
				opt.font        || '15px "Arial"'
			];
		}

		function handleOptions(self, opt, isNoFillStroke) {
			opt = opt || getOptions(self);
			context.globalAlpha = opt[3];
			context.globalCompositeOperation = opt[4];
			context.lineCap     = opt[5];
			context.lineJoin    = opt[6];
			context.lineWidth   = opt[0];
			context.strokeStyle = opt[1];
			context.fillStyle   = opt[2];
			context.font        = opt[7];
			if (!isNoFillStroke) {
				context.stroke();
				context.fill();
			}
		}

		var arrowSize = 10;
		var angle = Math.atan2(ly - my, lx - mx);
		context.beginPath();
		context.moveTo(mx, my);
		context.lineTo(lx, ly);
		handleOptions(self);
		context.beginPath();
		context.moveTo(lx, ly);
		context.lineTo(lx - arrowSize * Math.cos(angle - Math.PI / 7), ly - arrowSize * Math.sin(angle - Math.PI / 7));
		context.lineTo(lx - arrowSize * Math.cos(angle + Math.PI / 7), ly - arrowSize * Math.sin(angle + Math.PI / 7));
		context.lineTo(lx, ly);
		context.lineTo(lx - arrowSize * Math.cos(angle - Math.PI / 7), ly - arrowSize * Math.sin(angle - Math.PI / 7));
		handleOptions(self);
	} // drawArrow()

	function endLastPath(self) {
		if (self.tool == 'arc')
			arcHandler.end(self);
		else if (self.tool == 'quad')   quadHandler.end(self);
		else if (self.tool == 'bezier') bezierHandler.end(self);
		drawHelper.redraw(self);
		if (textHandler.text && textHandler.text.length) {
			textHandler.appendPoints(self);
			textHandler.onShapeUnSelected(self);
		}
//		textHandler.showOrHideTextTools(self, 'hide');
	}
	function copy(self) {
		var points = self.points;
		endLastPath(self);
		self.drag.startingIndex = 0;
		if (ID(self.obj,'copy-last').checked) {
			self.copiedStuff = points[points.length - 1];
			setSelection(ID(self.obj,'drag-last-path'), 'dragLast');
		} else {
			self.copiedStuff = points;
			setSelection(ID(self.obj,'drag-all-paths'), 'dragAll');
		}
	}
	function paste(self) {
		var points = self.points;
		endLastPath(self);
		self.drag.startingIndex = 0;
		if (ID(self.obj,'copy-last').checked) {
			points[points.length] = self.copiedStuff;
			self.drag = {
				prevX:0,
				prevY:0,
				startingIndex:points.length - 1
			};
			dragHelper.dragAllPaths(0, 0);
			setSelection(ID(self.obj,'drag-last-path'), 'dragLast');
		} else {
			self.drag.startingIndex = points.length;
			points = points.concat(copiedStuff);
			setSelection(ID(self.obj,'drag-all-paths'), 'dragAll');
		}
	}
	function onkeydown(self,e) {
		var keyCode = self.keyCode = e.which || e.keyCode || 0;
		if (keyCode == 8 || keyCode == 46) {
			if (isBackKey(e, keyCode)) {
				// back key pressed
			}
			return;
		}
		if (e.metaKey) {
			self.isControlKeyPressed = true;
			self.keyCode = 17;
		}
		if (!self.isControlKeyPressed && keyCode === 17)
			self.isControlKeyPressed = true;
	}

	function isBackKey(e, keyCode) {
		var doPrevent = false, d = e.srcElement || e.target;
		if ((d.tagName.toUpperCase() === 'INPUT'    &&
			(d.type.toUpperCase()    === 'TEXT'     ||
			 d.type.toUpperCase()    === 'PASSWORD' ||
			 d.type.toUpperCase()    === 'FILE'     ||
			 d.type.toUpperCase()    === 'SEARCH'   ||
			 d.type.toUpperCase()    === 'EMAIL'    ||
			 d.type.toUpperCase()    === 'NUMBER'   ||
			 d.type.toUpperCase()    === 'DATE'))   ||
			d.tagName.toUpperCase()  === 'TEXTAREA') {
			doPrevent = d.readOnly || d.disabled;
		} else {
			doPrevent = true;
		}
		if (doPrevent)
			e.preventDefault();
		return doPrevent;
	}

	function onkeyup(self,e) {
		var tool = self.tool, cKey = self.isControlKeyPressed, p = self.points;
		if (e.which == null && (e.charCode != null || e.keyCode != null))
			e.which = e.charCode != null ? e.charCode : e.keyCode;

		var keyCode = self.keyCode = (e.which || e.keyCode || 0);
		if (keyCode === 13 && tool=='text') {
			textHandler.onReturnKeyPressed(self);
			return;
		}
		if (keyCode == 8 || keyCode == 46) {
			if (isBackKey(e, keyCode))
				textHandler.writeText(textHandler.lastKeyPress, true);
			return;
		}
		// Ctrl + t
		if (cKey && keyCode === 84 && tool=='text') {
			textHandler.showTextTools();
			return;
		}
		// Ctrl + z
		if (cKey && keyCode === 90) {
			if (p.length) {
				p.length = p.length - 1;
				drawHelper.redraw(self);
//				syncPoints(tool == 'DragAllPaths' || tool == 'DragLastPath' ? true : false);
			}
		}
		// Ctrl + a
		if (cKey && keyCode === 65) {
			self.drag.startingIndex = 0;
			endLastPath(self);
			setSelection(ID(self.obj,'drag-all-paths'), 'dragAll');
		}
		// Ctrl + c
		if (cKey && keyCode === 67 && p.length)
			copy(self);
		// Ctrl + v
		if (cKey && keyCode === 86 && self.copiedStuff.length) {
			paste(self);
//				syncPoints(is.isDragAllPaths || is.isDragLastPath ? true : false);
		}
		// Ending the Control Key
		if (typeof e.metaKey !== 'undefined' && e.metaKey === false) {
			self.isControlKeyPressed = false;
			self.keyCode = 17;
		}
		if (self.keyCode === 17)
			self.isControlKeyPressed = false;
	}

	function onkeypress(self,e) {
		if (e.which == null && (e.charCode != null || e.keyCode != null))
			e.which = e.charCode != null ? e.charCode : e.keyCode;
		self.keyCode = e.which || e.keyCode || 0;
		if (/[a-zA-Z0-9-_ !?|\/'",.=:;(){}\[\]`~@#$%^&*+-]/.test(String.fromCharCode(self.keyCode)))
			textHandler.writeText(String.fromCharCode(self.keyCode));
	}
	function onTextFromClipboard(self,e) {
		if (!self.tool=='text')
			return;
		var pastedText;
		if (window.clipboardData && window.clipboardData.getData) { // IE
			pastedText = window.clipboardData.getData('Text');
		} else if (e.clipboardData && e.clipboardData.getData) {
			pastedText = e.clipboardData.getData('text/plain');
		}
		if (pastedText && pastedText.length)
			textHandler.writeText(pastedText);
	}
	function setTemporaryLine(self) {
		var arr = ["line", [139, 261, 170, 219],
			[1, "rgba(0,0,0,0)", "rgba(0,0,0,0)", 1, "source-over", "round", "round", "15px \"Arial\""]
		];
		self.points.push(arr);
		drawHelper.redraw(self);
		setTimeout(function() {
			setSelection(self, document.getElementById('line'), 'line');
		}, 1000);
//		setTimeout(setDefaultSelectedIcon, 2000);
	}

	/* Drag Functions */
	function isPointInPath(x, y, first, second) {
		return x > first - 10 && x < first + 10 && y > second - 10 && y < second + 10;
	}
	function getPoint2(point, prev, otherPoint) {
		if (point > prev) {
			point = otherPoint + (point - prev);
		} else {
			point = otherPoint - (prev - point);
		}
		return point;
	}
	function getXYWidthHeight(x, y, prevX, prevY, oldPoints){
		if (oldPoints.pointsToMove == 'stretch-first') {
			if (x > prevX) {
				oldPoints.x = oldPoints.x + (x - prevX);
				oldPoints.width = oldPoints.width - (x - prevX);
			} else {
				oldPoints.x = oldPoints.x - (prevX - x);
				oldPoints.width = oldPoints.width + (prevX - x);
			}
			if (y > prevY) {
				oldPoints.y = oldPoints.y + (y - prevY);
				oldPoints.height = oldPoints.height - (y - prevY);
			} else {
				oldPoints.y = oldPoints.y - (prevY - y);
				oldPoints.height = oldPoints.height + (prevY - y);
			}
		}

		if (oldPoints.pointsToMove == 'stretch-second') {
			if (x > prevX) {
				oldPoints.width = oldPoints.width + (x - prevX);
			} else {
				oldPoints.width = oldPoints.width - (prevX - x);
			}
			if (y < prevY) {
				oldPoints.y = oldPoints.y + (y - prevY);
				oldPoints.height = oldPoints.height - (y - prevY);
			} else {
				oldPoints.y = oldPoints.y - (prevY - y);
				oldPoints.height = oldPoints.height + (prevY - y);
			}
		}

		if (oldPoints.pointsToMove == 'stretch-third') {
			if (x > prevX) {
				oldPoints.x = oldPoints.x + (x - prevX);
				oldPoints.width = oldPoints.width - (x - prevX);
			} else {
				oldPoints.x = oldPoints.x - (prevX - x);
				oldPoints.width = oldPoints.width + (prevX - x);
			}
			if (y < prevY) {
				oldPoints.height = oldPoints.height + (y - prevY);
			} else {
				oldPoints.height = oldPoints.height - (prevY - y);
			}
		}
		return oldPoints;
	}

	/******************
	 * GLOBAL OBJECTS *
	 *****************/
	imageHandler = {
		load: function(self,width,height) {
			var t = self.image;
			self.points[self.points.length] = ['image', [t.lastImageURL, t.prevX, t.prevY, width, height, t.lastImageIndex], drawHelper.getOptions(self)];
			ID(self.obj,'drag-last-path').click();
			syncPoints(true);
		},
		mousedown:function(self,e) {
			var x   = e.pageX - self.canvas.offsetLeft,
				y   = e.pageY - self.canvas.offsetTop,
				t   = self.image;
			t.prevX = x;
			t.prevY = y;
			t.ismousedown = true;
		},
		mouseup:function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.image, points = self.points;
			if (t.ismousedown) {
				points[points.length] = ['image', [imageHandler.lastImageURL, t.prevX, t.prevY, x - t.prevX, y - t.prevY, imageHandler.lastImageIndex], drawHelper.getOptions(self)];
				t.ismousedown = false;
			}
		},
		mousemove: function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.image;
			if (t.ismousedown) {
				self.tempContext.clearRect(0, 0, self.width, self.height);
				drawHelper.image(self.tempContext, [imageHandler.lastImageURL, t.prevX, t.prevY, x - t.prevX, y - t.prevY, imageHandler.lastImageIndex]);
			}
		}
	};

	dragHelper = {
		mousedown:function(self,e) {
			if (self.isControlKeyPressed) {
				copy(self);
				paste(self);
				self.isControlKeyPressed = false;
			}
			var t   = self.drag, points = self.points,
				x   = e.pageX - self.canvas.offsetLeft,
				y   = e.pageY - self.canvas.offsetTop;
			t.prevX = x;
			t.prevY = y;
			t.pointsToMove = 'all';
			if (points.length) {
				var p = points[points.length - 1], point = p[1];
				if (p[0] === 'line') {
					if (isPointInPath(x, y, point[0], point[1]))
						t.pointsToMove = 'head';
					if (isPointInPath(x, y, point[2], point[3]))
						t.pointsToMove = 'tail';
				}
				if (p[0] === 'pencil') {
					if (isPointInPath(x, y, point[0], point[1]))
						t.pointsToMove = 'head';
					if (isPointInPath(x, y, point[2], point[3]))
						t.pointsToMove = 'tail';
				}
				if (p[0] === 'arrow') {
					if (isPointInPath(x, y, point[0], point[1]))
						t.pointsToMove = 'head';
					if (isPointInPath(x, y, point[2], point[3]))
						t.pointsToMove = 'tail';
				}
				if (p[0] === 'rect') {
					if (isPointInPath(x, y, point[0], point[1]))
						t.pointsToMove = 'stretch-first';
					if (isPointInPath(x, y, point[0] + point[2], point[1]))
						t.pointsToMove = 'stretch-second';
					if (isPointInPath(x, y, point[0], point[1] + point[3]))
						t.pointsToMove = 'stretch-third';
					if (isPointInPath(x, y, point[0] + point[2], point[1] + point[3]))
						t.pointsToMove = 'stretch-last';
				}
				if (p[0] === 'image') {
					if (isPointInPath(x, y, point[1], point[2]))
						t.pointsToMove = 'stretch-first';
					if (isPointInPath(x, y, point[1] + point[3], point[2]))
						t.pointsToMove = 'stretch-second';
					if (isPointInPath(x, y, point[1], point[2] + point[4]))
						t.pointsToMove = 'stretch-third';
					if (isPointInPath(x, y, point[1] + point[3], point[2] + point[4]))
						t.pointsToMove = 'stretch-last';
				}
				if (p[0] === 'pdf') {
					if (isPointInPath(x, y, point[1], point[2]))
						t.pointsToMove = 'stretch-first';
					if (isPointInPath(x, y, point[1] + point[3], point[2]))
						t.pointsToMove = 'stretch-second';
					if (isPointInPath(x, y, point[1], point[2] + point[4]))
						t.pointsToMove = 'stretch-third';
					if (isPointInPath(x, y, point[1] + point[3], point[2] + point[4]))
						t.pointsToMove = 'stretch-last';
				}
				if (p[0] === 'quadratic') {
					if (isPointInPath(x, y, point[0], point[1]))
						t.pointsToMove = 'starting-points';
					if (isPointInPath(x, y, point[2], point[3]))
						t.pointsToMove = 'control-points';
					if (isPointInPath(x, y, point[4], point[5]))
						t.pointsToMove = 'ending-points';
				}
				if (p[0] === 'bezier') {
					if (isPointInPath(x, y, point[0], point[1]))
						t.pointsToMove = 'starting-points';
					if (isPointInPath(x, y, point[2], point[3]))
						t.pointsToMove = '1st-control-points';
					if (isPointInPath(x, y, point[4], point[5]))
						t.pointsToMove = '2nd-control-points';
					if (isPointInPath(x, y, point[6], point[7]))
						t.pointsToMove = 'ending-points';
				}
			}
			g.ismousedown = true;
		},
		mouseup:function(self) {
			if (self.tool == 'dragLast') {
				self.tempContext.clearRect(0, 0, self.width, self.height);
				self.context.clearRect    (0, 0, self.width, self.height);
				dragHelper.end(self);
			}
			self.drag.ismousedown = false;
		},
		mousemove:function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop;
			drawHelper.redraw(self);
			if (self.drag.ismousedown)
				dragHelper.dragShape(x, y);
			if (self.tool == 'dragLast')
				dragHelper.init(self);
		},
		init:function(self) {
			var points = self.points, p, point, t = self.drag, c = self.tempContext;
			if (!points.length)
				return;
			p     = points[points.length - 1];
			point = p[1];

			if (t.ismousedown) c.fillStyle = 'rgba(255,85 ,154,.9)';
			else               c.fillStyle = 'rgba(255,85 ,154,.4)';

			if (p[0] === 'quadratic') {
				c.beginPath();
				c.arc(point[0], point[1], 10, Math.PI * 2, 0, !1);
				c.arc(point[2], point[3], 10, Math.PI * 2, 0, !1);
				c.arc(point[4], point[5], 10, Math.PI * 2, 0, !1);
				c.fill();
			}
			if (p[0] === 'bezier') {
				c.beginPath();
				c.arc(point[0], point[1], 10, Math.PI * 2, 0, !1);
				c.arc(point[2], point[3], 10, Math.PI * 2, 0, !1);
				c.arc(point[4], point[5], 10, Math.PI * 2, 0, !1);
				c.arc(point[6], point[7], 10, Math.PI * 2, 0, !1);
				c.fill();
			}
			if (p[0] === 'line') {
				c.beginPath();
				c.arc(point[0], point[1], 10, Math.PI * 2, 0, !1);
				c.arc(point[2], point[3], 10, Math.PI * 2, 0, !1);
				c.fill();
			}
			if (p[0] === 'pencil') {
				c.beginPath();
				c.arc(point[0], point[1], 10, Math.PI * 2, 0, !1);
				c.arc(point[2], point[3], 10, Math.PI * 2, 0, !1);
				c.fill();
			}
			if (p[0] === 'arrow') {
				c.beginPath();
				c.arc(point[0], point[1], 10, Math.PI * 2, 0, !1);
				c.arc(point[2], point[3], 10, Math.PI * 2, 0, !1);
				c.fill();
			}
			if (p[0] === 'text') {
				c.font = "15px Verdana";
				c.fillText(point[0], point[1], point[2]);
			}
			if (p[0] === 'rect') {
				c.beginPath();
				c.arc(point[0], point[1], 10, Math.PI * 2, 0, !1);
				c.fill();
				c.beginPath();
				c.arc(point[0] + point[2], point[1], 10, Math.PI * 2, 0, !1);
				c.fill();
				c.beginPath();
				c.arc(point[0], point[1] + point[3], 10, Math.PI * 2, 0, !1);
				c.fill();
				c.beginPath();
				c.arc(point[0] + point[2], point[1] + point[3], 10, Math.PI * 2, 0, !1);
				c.fill();
			}
			if (p[0] === 'image') {
				c.beginPath();
				c.arc(point[1], point[2], 10, Math.PI * 2, 0, !1);
				c.fill();
				c.beginPath();
				c.arc(point[1] + point[3], point[2], 10, Math.PI * 2, 0, !1);
				c.fill();
				c.beginPath();
				c.arc(point[1], point[2] + point[4], 10, Math.PI * 2, 0, !1);
				c.fill();
				c.beginPath();
				c.arc(point[1] + point[3], point[2] + point[4], 10, Math.PI * 2, 0, !1);
				c.fill();
			}
			if (p[0] === 'pdf') {
				c.beginPath();
				c.arc(point[1], point[2], 10, Math.PI * 2, 0, !1);
				c.fill();
				c.beginPath();
				c.arc(point[1] + point[3], point[2], 10, Math.PI * 2, 0, !1);
				c.fill();
				c.beginPath();
				c.arc(point[1], point[2] + point[4], 10, Math.PI * 2, 0, !1);
				c.fill();
				c.beginPath();
				c.arc(point[1] + point[3], point[2] + point[4], 10, Math.PI * 2, 0, !1);
				c.fill();
			}
		},
		dragShape:function(self, x, y) {
			if (!self.drag.ismousedown)
				return;
			self.tempContext.clearRect(0, 0, self.width, self.height);
			if (self.tool == 'dragLast')
				this.dragLastPath(self, x, y);
			if (self.tool == 'dragAll')
				this.dragAllPaths(self, x, y);
			self.drag.prevX = x;
			self.drag.prevY = y;
		},
		end:function(self) {
			if (!self.points.length)
				return;
			self.tempContext.clearRect(0, 0, self.width, self.height);
			var point = self.points[self.points.length - 1];
			drawHelper[point[0]](context, point[1], point[2]);
		},
		dragAllPaths:function(self,x, y) {
			var t      = self.drag,p,point,
				prevX  = t.prevX,
				prevY  = t.prevY,
				points = self.points,
				length = points.length,
				i      = t.startingIndex;

			for (i; i < length; i++) {
				p      = points[i];
				point  = p[1];
				if (p[0] === 'line') {
					points[i] = [p[0],
						[
							getPoint2(x, prevX, point[0]),
							getPoint2(y, prevY, point[1]),
							getPoint2(x, prevX, point[2]),
							getPoint2(y, prevY, point[3])
						], p[2]
					];
				}
				if (p[0] === 'pencil') {
					points[i] = [p[0],
						[
							getPoint2(x, prevX, point[0]),
							getPoint2(y, prevY, point[1]),
							getPoint2(x, prevX, point[2]),
							getPoint2(y, prevY, point[3])
						], p[2]
					];
				}
				if (p[0] === 'arrow') {
					points[i] = [p[0],
						[
							getPoint2(x, prevX, point[0]),
							getPoint2(y, prevY, point[1]),
							getPoint2(x, prevX, point[2]),
							getPoint2(y, prevY, point[3])
						], p[2]
					];
				}
				if (p[0] === 'text') {
					points[i] = [p[0],
						[
							point[0],
							getPoint(x, prevX, point[1]),
							getPoint(y, prevY, point[2])
						], p[2]
					];
				}
				if (p[0] === 'arc') {
					points[i] = [p[0],
						[
							getPoint(x, prevX, point[0]),
							getPoint(y, prevY, point[1]),
							point[2],
							point[3],
							point[4]
						], p[2]
					];
				}
				if (p[0] === 'rect') {
					points[i] = [p[0],
						[
							getPoint(x, prevX, point[0]),
							getPoint(y, prevY, point[1]),
							point[2],
							point[3]
						], p[2]
					];
				}
				if (p[0] === 'image') {
					points[i] = [p[0],
						[
							point[0],
							getPoint(x, prevX, point[1]),
							getPoint(y, prevY, point[2]),
							point[3],
							point[4],
							point[5]
						], p[2]
					];
				}
				if (p[0] === 'pdf') {
					points[i] = [p[0],
						[
							point[0],
							getPoint2(x, prevX, point[1]),
							getPoint2(y, prevY, point[2]),
							point[3],
							point[4],
							point[5]
						], p[2]
					];
				}
				if (p[0] === 'quadratic') {
					points[i] = [p[0],
						[
							getPoint2(x, prevX, point[0]),
							getPoint2(y, prevY, point[1]),
							getPoint2(x, prevX, point[2]),
							getPoint2(y, prevY, point[3]),
							getPoint2(x, prevX, point[4]),
							getPoint2(y, prevY, point[5])
						], p[2]
					];
				}

				if (p[0] === 'bezier') {
					points[i] = [p[0],
						[
							getPoint2(x, prevX, point[0]),
							getPoint2(y, prevY, point[1]),
							getPoint2(x, prevX, point[2]),
							getPoint2(y, prevY, point[3]),
							getPoint2(x, prevX, point[4]),
							getPoint2(y, prevY, point[5]),
							getPoint2(x, prevX, point[6]),
							getPoint2(y, prevY, point[7])
						], p[2]
					];
				}
			}
		},
		dragLastPath:function(self, x, y) {
			// if last past is undefined?
			var points = self.points;
			if (!points[points.length - 1]) 
				return;
			var t     = self.drag,
				prevX = t.prevX,
				prevY = t.prevY,
				p	  = points[points.length - 1],
				point = p[1],
				isMoveAllPoints = t.pointsToMove === 'all';

			if (p[0] === 'line') {
				if (t.pointsToMove === 'head' || isMoveAllPoints) {
					point[0] = getPoint(x, prevX, point[0]);
					point[1] = getPoint(y, prevY, point[1]);
				}
				if (t.pointsToMove === 'tail' || isMoveAllPoints) {
					point[2] = getPoint(x, prevX, point[2]);
					point[3] = getPoint(y, prevY, point[3]);
				}
				points[points.length - 1] = [p[0], point, p[2]];
			}

			if (p[0] === 'pencil') {
				if (t.pointsToMove === 'head' || isMoveAllPoints) {
					point[0] = getPoint(x, prevX, point[0]);
					point[1] = getPoint(y, prevY, point[1]);
				}
				if (t.pointsToMove === 'tail' || isMoveAllPoints) {
					point[2] = getPoint(x, prevX, point[2]);
					point[3] = getPoint(y, prevY, point[3]);
				}
				points[points.length - 1] = [p[0], point, p[2]];
			}
			if (p[0] === 'arrow') {
				if (t.pointsToMove === 'head' || isMoveAllPoints) {
					point[0] = getPoint(x, prevX, point[0]);
					point[1] = getPoint(y, prevY, point[1]);
				}
				if (t.pointsToMove === 'tail' || isMoveAllPoints) {
					point[2] = getPoint(x, prevX, point[2]);
					point[3] = getPoint(y, prevY, point[3]);
				}
				points[points.length - 1] = [p[0], point, p[2]];
			}
			if (p[0] === 'text') {
				if (t.pointsToMove === 'head' || isMoveAllPoints) {
					point[1] = getPoint(x, prevX, point[1]);
					point[2] = getPoint(y, prevY, point[2]);
				}
				points[points.length - 1] = [p[0], point, p[2]];
			}
			if (p[0] === 'arc') {
				point[0] = getPoint(x, prevX, point[0]);
				point[1] = getPoint(y, prevY, point[1]);
				points[points.length - 1] = [p[0], point, p[2]];
			}
			if (p[0] === 'rect') {
				if (isMoveAllPoints) {
					point[0] = getPoint(x, prevX, point[0]);
					point[1] = getPoint(y, prevY, point[1]);
				}
				if (t.pointsToMove === 'stretch-first') {
					var newPoints = getXYWidthHeight(x, y, prevX, prevY, {
						x:point[0],
						y:point[1],
						width:point[2],
						height:point[3],
						pointsToMove:t.pointsToMove
					});
					point[0] = newPoints.x;
					point[1] = newPoints.y;
					point[2] = newPoints.width;
					point[3] = newPoints.height;
				}
				if (t.pointsToMove === 'stretch-second') {
					var newPoints = getXYWidthHeight(x, y, prevX, prevY, {
						x:point[0],
						y:point[1],
						width:point[2],
						height:point[3],
						pointsToMove:t.pointsToMove
					});
					point[1] = newPoints.y;
					point[2] = newPoints.width;
					point[3] = newPoints.height;
				}
				if (t.pointsToMove === 'stretch-third') {
					var newPoints = getXYWidthHeight(x, y, prevX, prevY, {
						x:point[0],
						y:point[1],
						width:point[2],
						height:point[3],
						pointsToMove:t.pointsToMove
					});
					point[0] = newPoints.x;
					point[2] = newPoints.width;
					point[3] = newPoints.height;
				}
				if (t.pointsToMove === 'stretch-last') {
					point[2] = getPoint(x, prevX, point[2]);
					point[3] = getPoint(y, prevY, point[3]);
				}
				points[points.length - 1] = [p[0], point, p[2]];
			}
			if (p[0] === 'image') {
				if (isMoveAllPoints) {
					point[1] = getPoint(x, prevX, point[1]);
					point[2] = getPoint(y, prevY, point[2]);
				}
				if (t.pointsToMove === 'stretch-first') {
					var newPoints = getXYWidthHeight(x, y, prevX, prevY, {
						x:point[1],
						y:point[2],
						width:point[3],
						height:point[4],
						pointsToMove:t.pointsToMove
					});
					point[1] = newPoints.x;
					point[2] = newPoints.y;
					point[3] = newPoints.width;
					point[4] = newPoints.height;
				}
				if (t.pointsToMove === 'stretch-second') {
					var newPoints = getXYWidthHeight(x, y, prevX, prevY, {
						x:point[1],
						y:point[2],
						width:point[3],
						height:point[4],
						pointsToMove:t.pointsToMove
					});
					point[2] = newPoints.y;
					point[3] = newPoints.width;
					point[4] = newPoints.height;
				}
				if (t.pointsToMove === 'stretch-third') {
					var newPoints = getXYWidthHeight(x, y, prevX, prevY, {
						x:point[1],
						y:point[2],
						width:point[3],
						height:point[4],
						pointsToMove:t.pointsToMove
					});
					point[1] = newPoints.x;
					point[3] = newPoints.width;
					point[4] = newPoints.height;
				}
				if (t.pointsToMove === 'stretch-last') {
					point[3] = getPoint(x, prevX, point[3]);
					point[4] = getPoint(y, prevY, point[4]);
				}
				points[points.length - 1] = [p[0], point, p[2]];
			}
			if (p[0] === 'pdf') {
				if (isMoveAllPoints) {
					point[1] = getPoint(x, prevX, point[1]);
					point[2] = getPoint(y, prevY, point[2]);
				}
				if (t.pointsToMove === 'stretch-first') {
					var newPoints = getXYWidthHeight(x, y, prevX, prevY, {
						x:point[1],
						y:point[2],
						width:point[3],
						height:point[4],
						pointsToMove:t.pointsToMove
					});
					point[1] = newPoints.x;
					point[2] = newPoints.y;
					point[3] = newPoints.width;
					point[4] = newPoints.height;
				}
				if (t.pointsToMove === 'stretch-second') {
					var newPoints = getXYWidthHeight(x, y, prevX, prevY, {
						x:point[1],
						y:point[2],
						width:point[3],
						height:point[4],
						pointsToMove:t.pointsToMove
					});
					point[2] = newPoints.y;
					point[3] = newPoints.width;
					point[4] = newPoints.height;
				}
				if (t.pointsToMove === 'stretch-third') {
					var newPoints = getXYWidthHeight(x, y, prevX, prevY, {
						x:point[1],
						y:point[2],
						width:point[3],
						height:point[4],
						pointsToMove:t.pointsToMove
					});
					point[1] = newPoints.x;
					point[3] = newPoints.width;
					point[4] = newPoints.height;
				}
				if (t.pointsToMove === 'stretch-last') {
					point[3] = getPoint(x, prevX, point[3]);
					point[4] = getPoint(y, prevY, point[4]);
				}
				points[points.length - 1] = [p[0], point, p[2]];
			}
			if (p[0] === 'quadratic') {
				if (t.pointsToMove === 'starting-points' || isMoveAllPoints) {
					point[0] = getPoint(x, prevX, point[0]);
					point[1] = getPoint(y, prevY, point[1]);
				}
				if (t.pointsToMove === 'control-points' || isMoveAllPoints) {
					point[2] = getPoint(x, prevX, point[2]);
					point[3] = getPoint(y, prevY, point[3]);
				}
				if (t.pointsToMove === 'ending-points' || isMoveAllPoints) {
					point[4] = getPoint(x, prevX, point[4]);
					point[5] = getPoint(y, prevY, point[5]);
				}
				points[points.length - 1] = [p[0], point, p[2]];
			}
			if (p[0] === 'bezier') {
				if (t.pointsToMove === 'starting-points' || isMoveAllPoints) {
					point[0] = getPoint(x, prevX, point[0]);
					point[1] = getPoint(y, prevY, point[1]);
				}
				if (t.pointsToMove === '1st-control-points' || isMoveAllPoints) {
					point[2] = getPoint(x, prevX, point[2]);
					point[3] = getPoint(y, prevY, point[3]);
				}
				if (t.pointsToMove === '2nd-control-points' || isMoveAllPoints) {
					point[4] = getPoint(x, prevX, point[4]);
					point[5] = getPoint(y, prevY, point[5]);
				}
				if (t.pointsToMove === 'ending-points' || isMoveAllPoints) {
					point[6] = getPoint(x, prevX, point[6]);
					point[7] = getPoint(y, prevY, point[7]);
				}
				points[points.length - 1] = [p[0], point, p[2]];
			}
		}
	};

	pencilHandler = {
		mousedown:function(self,e) {
			var x   = e.pageX - self.canvas.offsetLeft,
				y   = e.pageY - self.canvas.offsetTop,
				t   = self.pencil;
			t.prevX = x;
			t.prevY = y;
			t.ismousedown = true;
			// make sure that pencil is drawing shapes even if mouse is down but mouse isn't moving
			self.tempContext.lineCap = 'round';
			drawHelper.pencil(self, self.tempContext,    [t.prevX, t.prevY, x, y]);
			self.points[self.points.length] = ['pencil', [t.prevX, t.prevY, x, y], pencilHandler.ops(self), 'start'];
			t.prevX = x;
			t.prevY = y;
		},
		mouseup:function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.pencil;
			if (t.ismousedown) {
				self.tempContext.lineCap = 'round';
				drawHelper.pencil(self, self.tempContext,    [t.prevX, t.prevY, x, y]);
				self.points[self.points.length] = ['pencil', [t.prevX, t.prevY, x, y], pencilHandler.ops(self), 'end'];
				t.prevX = x;
				t.prevY = y;
			}
			this.ismousedown = false;
		},
		mousemove:function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.pencil;
			if (t.ismousedown) {
				self.tempContext.lineCap = 'round';
				drawHelper.pencil(self, self.tempContext,    [t.prevX, t.prevY, x, y]);
				self.points[self.points.length] = ['pencil', [t.prevX, t.prevY, x, y], pencilHandler.ops(self)];
				t.prevX = x;
				t.prevY = y;
			}
		},
		ops:function(o) {
			return [o.pencilLineWidth, o.pencilStrokeStyle, o.fillStyle, o.globalAlpha, o.globalCompositeOperation, o.lineCap, o.lineJoin, o.font]
		}
	}

	markerHandler = {
		mousedown:function(self,e) {
			var x   = e.pageX - self.canvas.offsetLeft,
				y   = e.pageY - self.canvas.offsetTop,
				t   = self.marker;
			t.prevX = x;
			t.prevY = y;
			t.ismousedown = true;
			// make sure that pencil is drawing shapes even if mouse is down but mouse isn't moving
			self.tempContext.lineCap = 'round';
			drawHelper.line(self, self.tempContext,    [t.prevX, t.prevY, x, y]);
			self.points[self.points.length] = ['line', [t.prevX, t.prevY, x, y], markerHandler.ops(self)];
			t.prevX = x;
			t.prevY = y;
		},
		mouseup:function(self) {
			self.marker.ismousedown = false;
		},
		mousemove:function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.marker;

			if (t.ismousedown) {
				self.tempContext.lineCap = 'round';
				drawHelper.line(self, self.tempContext,    [t.prevX, t.prevY, x, y]);
				self.points[self.points.length] = ['line', [t.prevX, t.prevY, x, y], markerHandler.ops(self)];
				t.prevX = x;
				t.prevY = y;
			}
//			console.log("move moving marker: " + t.prevX, + " " + t.prevY + " " +x + " " + y);
		},
		ops:function(o) {
			return [o.markerLineWidth, o.markerStrokeStyle, o.fillStyle, o.markerGlobalAlpha, o.globalCompositeOperation, o.lineCap, o.lineJoin, o.font];
		}
	}
	
	textHandler = {
		onShapeSelected: function(self) {
			var t = self.text;
			self.tempContext.canvas.style.cursor = 'text';
			t.x = t.y = t.pageX = t.pageY = 0;
			t.text = '';
		},
		onShapeUnSelected: function(self) {
			var t  = self.text;
			t.text = '';
			t.showOrHideTextTools('hide');
			self.tempContext.canvas.style.cursor = 'default';
			if (typeof t.blinkCursorInterval !== 'undefined')
				clearInterval(t.blinkCursorInterval);
		},
		getFillColor:function(color) {
			color = (color || fillStyle).toLowerCase();
			if (color == 'rgba(255, 255, 255, 0)' || color == 'transparent' || color === 'white')
				return 'black';
			return color;
		},
		writeText:function(keyPressed, isBackKeyPressed) {
			var t = self.text;
			if (self.tool != 'text')
				return;	
			if (self.isBackKeyPressed) {
				t.text = t.text.substr(0, t.text.length - 1);
				textHandler.fillText(self,t.text);
				return;
			}	
			t.text += keyPressed;
			textHandler.fillText(t.text);
		},
		fillText:function(self,text) {
			var c = self.tempContext, t = self.text;
			if (self.tool != 'text')
				return;
			self.tempContext.clearRect(0, 0, tempContext.canvas.width, tempContext.canvas.height);	
			var options = textHandler.getOptions(self);
			drawHelper.handleOptions(self, c, options);
			c.fillStyle = textHandler.getFillColor(options[2]);
			c.font      = t.selectedFontSize + 'px "' + t.selectedFontFamily + '"';
			c.fillText(t.text, t.x, t.y);
		},
		blinkCursor:function(self) {
			var t = self.text;
			t.index++;
			if (t.index % 2 == 0) {
				textHandler.fillText(self,t.text + '|');
			} else {
				textHandler.fillText(self,t.text);
			}
		},
		getOptions:function(self) {
			var t = self.text, options = {
				font:t.selectedFontSize + 'px "' + t.selectedFontFamily + '"',
				fillStyle:textHandler.getFillColor(),
				strokeStyle:'#6c96c8',
				globalCompositeOperation:'source-over',
				globalAlpha:1,
				lineJoin:'round',
				lineCap:'round',
				lineWidth:2
			};
			t.font = options.font;
			return options;
		},
		appendPoints:function(self) {
			var t = self.text;
			self.points[self.points.length] = ['text', ['"' + t.text + '"', t.x, t.y], drawHelper.getOptions(self, textHandler.getOptions(self))];
		},
		mousedown:function(e) {
			var t = self.text;
			if (self.tool != 'text')
				return;
			if (t.text.length)
				textHandler.appendPoints(self);	
			t.x	    = textHandler.y = 0;
			t.text  = '';
			t.pageX = e.pageX;
			t.pageY = e.pageY;	
			t.x	    = e.pageX - self.canvas.offsetLeft - 5;
			t.y	    = e.pageY - self.canvas.offsetTop + 10;
			if (typeof t.blinkCursorInterval !== 'undefined')
				clearInterval(t.blinkCursorInterval);
			textHandler.blinkCursor();
			textHandler.blinkCursorInterval = setInterval(textHandler.blinkCursor, 700);	
			textHandler.showTextTools();
		},
		mouseup:function()   {},
		mousemove:function() {},
		showOrHideTextTools: function(self,show) {
			var t = self.text;
			if (show === 'hide') {
				if (t.lastFillStyle.length) {
					self.fillStyle = t.lastFillStyle;
					t.lastFillStyle = '';
				}
			} else if (!t.lastFillStyle.length) {
				t.lastFillStyle = self.fillStyle;
				self.fillStyle = 'black';
			}	
			t.fontFamilyBox.style.display = show == 'show' ? 'block' : 'none';
			t.fontSizeBox.style.display   = show == 'show' ? 'block' : 'none';
			t.fontSizeBox.style.left      = t.x + 'px';
			t.fontFamilyBox.style.left    = (t.fontSizeBox.clientWidth + t.x) + 'px';
			t.fontSizeBox.style.top       = t.y + 'px';
			t.fontFamilyBox.style.top     = t.y + 'px';
		},
		showTextTools: function(self) {
			var t = self.text;
			if (!t.fontFamilyBox || !t.fontSizeBox)
				return;
			textHandler.unselectAllFontFamilies();
			textHandler.unselectAllFontSizes();	
			textHandler.showOrHideTextTools('show');
			textHandler.eachFontFamily(function(child) {
				child.onclick = function(e) {
					e.preventDefault();	
					textHandler.showOrHideTextTools('hide');
					textHandler.selectedFontFamily = this.innerHTML;
					this.className = 'font-family-selected';
				};
				child.style.fontFamily = child.innerHTML;
			});	
			this.eachFontSize(function(child) {
				child.onclick = function(e) {
					e.preventDefault();
					textHandler.showOrHideTextTools('hide');	
					textHandler.selectedFontSize = this.innerHTML;
					this.className = 'font-family-selected';
				};
				// child.style.fontSize = child.innerHTML + 'px';
			});
		},
		eachFontFamily: function(callback) {
			var childs = this.fontFamilyBox.querySelectorAll('li');
			for (var i = 0; i < childs.length; i++) {
				callback(childs[i]);
			}
		},
		unselectAllFontFamilies: function() {
			this.eachFontFamily(function(child) {
				child.className = '';
				if (child.innerHTML === textHandler.selectedFontFamily) {
					child.className = 'font-family-selected';
				}
			});
		},
		eachFontSize: function(callback) {
			var childs = this.fontSizeBox.querySelectorAll('li');
			for (var i = 0; i < childs.length; i++) {
				callback(childs[i]);
			}
		},
		unselectAllFontSizes: function() {
			this.eachFontSize(function(child) {
				child.className = '';
				if (child.innerHTML === textHandler.selectedFontSize) {
					child.className = 'font-size-selected';
				}
			});
		},
		onReturnKeyPressed: function(self) {
			if (!textHandler.text || !textHandler.text.length) return;
			var fontSize = parseInt(textHandler.selectedFontSize) || 15;
			this.mousedown({
				pageX: this.pageX,
				// pageY: parseInt(tempContext.measureText(textHandler.text).height * 2) + 10
				pageY: this.pageY + fontSize + 5
			});
			drawHelper.redraw(self);
		},
		fontFamilyBox:document.querySelector('.fontSelectUl'),
		fontSizeBox:document.querySelector('.fontSizeUl')
	};
	
	arcHandler = {
		mousedown:function(self,e) {
			var t  = self.arc,
				x  = e.pageX - self.canvas.offsetLeft,
				y  = e.pageY - self.canvas.offsetTop;
				t.prevX       = x;
				t.prevY       = y;	
				t.ismousedown = true;
		},
		mouseup:function(self,e) {
			var t = self.arc,
				x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop;
			if (t.ismousedown) {
				if (!t.isCircleDrawn && t.isCircledEnded) {
					var prevX  = t.prevX,
						prevY  = t.prevY,
						radius = ((x - prevX) + (y - prevY)) / 3,
						c      = (2 * Math.PI * radius) / 21,
						xx     = prevX > x ? prevX - x : x - prevX,
						yy     = prevY > y ? prevY - y : y - prevY,
						angle;

					t.prevRadius          = radius;
					t.isCircleDrawn       = true,
					t.isCircleEnded       = false,
					angle                 = (xx + yy) / (2 * c);
					points[points.length] = ['arc', [prevX + radius, prevY + radius, radius, angle, 1], drawHelper.getOptions(self)];

					t.arcRangeContainer.style.display = 'block';
					t.arcRange.focus();
					t.arcRangeContainer.style.top = (y + canvas.offsetTop + 20) + 'px';
					t.arcRangeContainer.style.left = x + 'px';
					t.arcRange.value = 2;
				} else if (t.isCircleDrawn && !t.isCircleEnded) {
					arcHandler.end(self);
				}
			}
			t.ismousedown = false;
			arcHandler.fixAllPoints(self);
		},
		mousemove:function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.arc,
				ismousedown   = t.ismousedown,
				isCircleDrawn = t.isCircleDrawn,
				isCircleEnded = t.isCircledEnded;

			if (ismousedown) {
				if (!isCircleDrawn && isCircleEnded) {
					var prevX  = t.prevX,
						prevY  = t.prevY,
						radius = ((x - prevX) + (y - prevY)) / 3;
					self.tempContext.clearRect(0, 0, 2000, 2000);
					drawHelper.arc(self, self.tempContext, [prevX + radius, prevY + radius, radius, Math.PI * 2, true]);
				}
			}
		},
		fixAllPoints:function(self) {
			var points = self.points, point, p;
			for (var i = 0; i < points.length; i++) {
				p = points[i];
				if (p[0] === 'arc') {
					point = p[1];
					points[i] = ['arc', [toFixed(point[0]), toFixed(point[1]), toFixed(point[2]), toFixed(point[3]), point[4]], p[2]];
				}
			}
		},
		init:function(self) {
			var t = self.arc,GID = self.GID, point, points = self.points, markIsClockwise = ID(self.obj,'is-clockwise');
			t.arcRangeContainer  = ID(self.obj,'arc-range-container');
			t.arcRange           = ID(self.obj,'arc-range');
			addEvent(markIsClockwise, 'change', function(e) {
				t.isClockwise    = markIsClockwise.checked;	
				t.arcRange.value = toFixed(t.arcRange.value);
				t.arcRange.focus();
				arcHandler.arcRangeHandler(self,e);
				if (!points.length)
					return;
				point = points[points.length - 1][1];
				self.tempContext.clearRect(0, 0, self.width, self.height);
				drawHelper.arc(self, self.tempContext, [point[0], point[1], point[2], point[3], point[4]]);
			});
			addEvent(t.arcRange, 'keydown', function(e){arcHandler.arcRangeHandler(self,e)});
			addEvent(t.arcRange, 'focus',   function(e){arcHandler.arcRangeHandler(self,e)});
		},
		arcRangeHandler:function(self,e) {
			var t        = self.arc,
				arcRange = t.arcRange,
				key      = e.keyCode,
				value    = +arcRange.value;
			if (key == 39 || key == 40) arcRange.value = (value < 2 ? value : 1.98) + .02;
			if (key == 37 || key == 38) arcRange.value = (value > 0 ? value : .02)  - .02;

			if (!key || key == 13 || key == 39 || key == 40 || key == 37 || key == 38) {
				var range = Math.PI * toFixed(value),
				points    = self.points,
				p         = points[points.length - 1];
				if (p[0] === 'arc') {
					var point = p[1];
					points[points.length - 1] = ['arc', [point[0], point[1], point[2], range, t.isClockwise ? 1 : 0], p[2]];
					drawHelper.redraw(self);
				}
			}
		},
		end:function(self) {
			var t = self.arc;
			t.arcRangeContainer.style.display = 'none';
			t.arcRange.value = 2;
			t.isCircleDrawn  = false;
			t.isCircleEnded  = true;	
			drawHelper.redraw(self);
		}
	};

	lineHandler = {
		mousedown: function(self,e) {
			var x   = e.pageX - self.canvas.offsetLeft,
				y   = e.pageY - self.canvas.offsetTop,
				t   = self.line;
			t.prevX = x;
			t.prevY = y;	
			t.ismousedown = true;
		},
		mouseup: function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.line;
			if (t.ismousedown) {
				self.points[self.points.length] = ['line', [t.prevX, t.prevY, x, y], drawHelper.getOptions(self)];	
				t.ismousedown = false;
			}
		},
		mousemove:function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.line;
			if (t.ismousedown) {
				self.tempContext.clearRect(0, 0, self.width, self.height);
				drawHelper.line(self.tempContext, [t.prevX, t.prevY, x, y]);
			}
		}
	};

	arrowHandler = {
		mousedown: function(self,e) {
			var x   = e.pageX - self.canvas.offsetLeft,
				y   = e.pageY - self.canvas.offsetTop,
				t   = self.arrow;
			t.prevX = x;
			t.prevY = y;	
			t.ismousedown = true;
		},
		mouseup:function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.arrow;
			if (t.ismousedown) {
				self.points[self.points.length] = ['arrow', [t.prevX, t.prevY, x, y], drawHelper.getOptions(self)];	
				t.ismousedown = false;
			}
		},
		mousemove: function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.arrow;	
			if (t.ismousedown) {
				self.tempContext.clearRect(0, 0, self.width, self.height);
				drawHelper.arrow(self.tempContext, [t.prevX, t.prevY, x, y]);
			}
		}
	};

	rectHandler = {
		mousedown: function(self,e) {
			var x   = e.pageX - self.canvas.offsetLeft,
				y   = e.pageY - self.canvas.offsetTop,
				t   = self.rect;
			t.prevX = x;
			t.prevY = y;	
			t.ismousedown = true;
		},
		mouseup: function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.rect;
			if (t.ismousedown) {
				self.points[self.points.length] = ['rect', [t.prevX, t.prevY, x - t.prevX, y - t.prevY], drawHelper.getOptions(self)];
				t.ismousedown = false;
			}

		},
		mousemove:function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.rect;
			if (t.ismousedown) {
				self.tempContext.clearRect(0, 0, self.width, self.height);
				drawHelper.rect(self.tempContext, [t.prevX, t.prevY, x - t.prevX, y - t.prevY]);
			}
		}
	};

	quadHandler = {
		mousedown: function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.quad;
			if (!t.isLastStep) {
				t.prevX = x;
				t.prevY = y;
			}
			t.ismousedown = true;
			if (t.isLastStep && t.ismousedown)
				quadHandler.end(self, x, y);
		},
		mouseup:function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.quad;
			if (t.ismousedown && t.isFirstStep) {
				t.controlPointX = x;
				t.controlPointY = y;
				t.isFirstStep   = false;
				t.isLastStep    = true;
			}
		},
		mousemove:function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.quad, c = self.tempContext;
			c.clearRect(0, 0, self.width, self.height);
			if (t.ismousedown && t.isFirstStep)
				drawHelper.quadratic(self, c, [t.prevX, t.prevY, x, y, x, y]);
			if (t.isLastStep)
				drawHelper.quadratic(self, c, [t.prevX, t.prevY, t.controlPointX, t.controlPointY, x, y]);
		},
		end:function(self,x,y) {
			var t = self.quad;
			if (!t.ismousedown)
				return;
			t.isLastStep  = false;
			t.isFirstStep = true;
			t.ismousedown = false;
			x = x || t.controlPointX || t.prevX;
			y = y || t.controlPointY || t.prevY;
			self.points[self.points.length] = ['quadratic', [t.prevX, t.prevY, t.controlPointX, t.controlPointY, x, y], drawHelper.getOptions(self)];
		}
	};

	bezierHandler = {
		mousedown: function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.bezier;
			if (!t.isLastStep && !t.isSecondStep) {
				t.prevX = x;
				t.prevY = y;
			}
			t.ismousedown = true;
			if (t.isLastStep && t.ismousedown)
				bezierHandler.end(self, x, y);
			if (t.ismousedown && t.isSecondStep) {
				t.secondControlPointX = x;
				t.secondControlPointY = y;
				t.isSecondStep        = false;
				t.isLastStep          = true;
			}
		},
		mouseup:function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.bezier;
			if (t.ismousedown && t.isFirstStep) {
				t.firstControlPointX = x;
				t.firstControlPointY = y;
				t.isFirstStep        = false;
				t.isSecondStep       = true;
			}
		},
		mousemove:function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.bezier, c = self.tempContext;
			c.clearRect(0, 0, self.width, self.height);
			if (t.ismousedown && t.isFirstStep)
				drawHelper.bezier(self, c, [t.prevX, t.prevY, x, y, x, y, x, y]);
			if (t.ismousedown && t.isSecondStep)
				drawHelper.bezier(self, c, [t.prevX, t.prevY, t.firstControlPointX, t.firstControlPointY, x, y, x, y]);
			if (t.isLastStep)
				drawHelper.bezier(self, c, [t.prevX, t.prevY, t.firstControlPointX, t.firstControlPointY, t.secondControlPointX, t.secondControlPointY, x, y]);
		},
		end:function(self, x, y) {
			var t = self.bezier;
			if (!t.ismousedown)
				return;
			t.isLastStep  = t.isSecondStep = false;
			t.isFirstStep = true;
			t.ismousedown = false;
			t.secondControlPointX = t.secondControlPointX || t.firstControlPointX;
			t.secondControlPointY = t.secondControlPointY || t.firstControlPointY;
			x = x || t.secondControlPointX;
			y = y || t.secondControlPointY;
			self.points[self.points.length] = ['bezier', [t.prevX, t.prevY, t.firstControlPointX, t.firstControlPointY, t.secondControlPointX, t.secondControlPointY, x, y], drawHelper.getOptions(self)];
		}
	};

	zoomHandler = {
		up:function(self,e) {
			self.zoom.scale += .01;
			zoomHandler.apply(self);
		},
		down:function(self,e) {
			self.zoom.scale -= .01;
			zoomHandler.apply();
		},
		apply:function(self) {
			var scale = self.zoom.scale;
			self.tempContext.scale(scale, scale);
			self.context.scale(scale, scale);
			drawHelper.redraw(self);
		},
		icons: {
			up:function(ctx) {
				ctx.font = '22px Verdana';
				ctx.strokeText('+', 10, 30);
			},
			down:function(ctx) {
				ctx.font = '22px Verdana';
				ctx.strokeText('-', 15, 30);
			}
		}
	};

	eraserHandler = {
		mousedown:function(self,e) {
			var x   = e.pageX - self.canvas.offsetLeft,
				y   = e.pageY - self.canvas.offsetTop,
				t   = self.eraser, points = self.points;
			t.prevX = x;
			t.prevY = y;
			t.ismousedown = true;
			self.tempContext.lineCap = 'round';
			drawHelper.line(self, self.tempContext, [t.prevX, t.prevY, x, y]);
			points[points.length]  = ['line',       [t.prevX, t.prevY, x, y], drawHelper.getOptions(self)];
			t.prevX = x;
			t.prevY = y;
		},
		mouseup:function(self,e) {
			self.eraser.ismousedown = false;
		},
		mousemove:function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = this, points = self.points;
			if (t.ismousedown) {
				self.tempContext.lineCap = 'round';
				drawHelper.line(self, self.tempContext, [t.prevX, t.prevY, x, y]);
				points[points.length]  = ['line',       [t.prevX, t.prevY, x, y], drawHelper.getOptions(self)];
				t.prevX = x;
				t.prevY = y;
			}
		}
	};
	pdfHandler = {
		getPage: function(self, pageNumber, callback) {
			var t = self.pdf;
			pageNumber = parseInt(pageNumber) || 1;
			if (!t.pdf) {
				pdfjsLib.disableWorker = false;
				pdfjsLib.getDocument(t.lastPdfURL).then(function(pdf) {
					t.pdf = pdf;
					pdfHandler.getPage(self, pageNumber, callback);
				});
				return;
			}

			t.pdf.getPage(pageNumber).then(function(page) {
				var scale         = 1.0,
					viewport      = page.getViewport(scale),
					canvas        = document.createElement('canvas'),
					ctx           = canvas.getContext('2d');
					canvas.height = viewport.height,
					canvas.width  = viewport.width,
					renderContext = {
						canvasContext:ctx,
						viewport:viewport
					};
					CVX = canvas;
					t.pageNumber  = pageNumber;
				if (t.removeWhiteBackground === true)
					renderContext.background = 'rgba(0,0,0,0)';
				page.render(renderContext).then(function() {
					if (t.removeWhiteBackground === true) {
						var imgd = ctx.getImageData(0, 0, canvas.width, canvas.height),
						pix      = imgd.data,
						newColor = {r:0,g:0,b:0,a:0};

						for (var i = 0, n = pix.length; i < n; i += 4) {
							var r = pix[i],
								g = pix[i + 1],
								b = pix[i + 2];
							if (r == 255 && g == 255 && b == 255) {
								pix[i]     = newColor.r;
								pix[i + 1] = newColor.g;
								pix[i + 2] = newColor.b;
								pix[i + 3] = newColor.a;
							}
						}
						ctx.putImageData(imgd, 0, 0);
					}
					t.lastPage = canvas.toDataURL('image/png');
					callback(t.lastPage, canvas.width, canvas.height, t.pdf.numPages);
				});
			});
		},
		load:function(self, lastPdfURL) {
			var t = self.pdf;
			t.lastPdfURL = lastPdfURL;
			pdfHandler.getPage(self, parseInt(t.pdfPagesList.value || 1), function(lastPage, width, height, numPages) {
				t.prevX                  = self.canvas.width - width - parseInt(width / 2);
				t.lastIndex              = t.images.length;
				var point                = [lastPage, 60, 20, width, height, t.lastIndex], points = self.points;
				t.lastPointIndex         = points.length;
				points[points.length]	 = ['pdf', point, drawHelper.getOptions(self)];
				t.pdfPagesList.innerHTML = '';
				for (var i = 1; i <= numPages; i++) {
					var option       = document.createElement('option');
					option.value     = i;
					option.innerHTML = 'Page #' + i;
					t.pdfPagesList.appendChild(option);
					if (t.pageNumber.toString() == i.toString())
						option.selected = true;
				}
				t.pdfPagesList.onchange = function() {pdfHandler.load(self, lastPdfURL)};
				t.pdfClose.onclick      = function() {t.pdfPageContainer.style.display = 'none'};
				t.pdfNext.onclick       = function() {
					t.pdfPagesList.selectedIndex++;
					t.pdfPagesList.onchange();
				};
				t.pdfPrev.onclick       = function() {
					t.pdfPagesList.selectedIndex--;
					t.pdfPagesList.onchange();
				};
//				ID(self.obj,'drag-last-path').click();
				t.pdfPrev.src  = data_uris.pdf_next;
				t.pdfNext.src  = data_uris.pdf_prev;
				t.pdfClose.src = data_uris.pdf_close;
				t.pdfPageContainer.style.top     = '20px';
				t.pdfPageContainer.style.left    = (point[3] - parseInt(point[3] / 2)) + 'px';
				t.pdfPageContainer.style.display = 'block';
//				syncPoints(true);
			});
		},
		mousedown:function(self,e) {
			var x   = e.pageX - self.canvas.offsetLeft,
				y   = e.pageY - self.canvas.offsetTop,
				t   = self.pdf;
			t.prevX = x;
			t.prevY = y;
			t.ismousedown = true;
		},
		mouseup:function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.pdf;
			if (t.ismousedown) {
				if (self.points[t.lastPointIndex])
					self.points[t.lastPointIndex] = ['pdf', [t.lastPage, t.prevX, t.prevY, x - t.prevX, y - t.prevY, t.lastIndex], drawHelper.getOptions(self)];
				t.ismousedown = false;
			}
		},
		mousemove:function(self,e) {
			var x = e.pageX - self.canvas.offsetLeft,
				y = e.pageY - self.canvas.offsetTop,
				t = self.pdf;
			if (t.ismousedown) {
				self.tempContext.clearRect(0, 0, self.width, self.height);
				drawHelper.pdf(self, self.tempContext, [t.lastPage, t.prevX, t.prevY, x - t.prevX, y - t.prevY, t.lastIndex]);
			}
		},
		reset_pos:function(self,x,y) {
			var t = self.pdf, points = self.points;
			t.pdfPageContainer.style.top = y + 'px';
			if (!points[t.lastPointIndex]) return;
			var point = points[t.lastPointIndex][1];
			t.pdfPageContainer.style.left = (point[1] + point[3] - parseInt(point[3] / 2) - parseInt(t.pdfPageContainer.clientWidth / 2)) + 'px';
		},
		end: function() {
			// pdfHandler.pdfPageContainer.style.display = 'none';
		}
	};

	drawHelper = {
		redraw:function(self) {
			self.tempContext.clearRect(0, 0, self.width, self.height);
			self.context.clearRect    (0, 0, self.width, self.height);
			var i, points = self.points, point, length = points.length;
			for (i = 0; i < length; i++) {
				point = points[i];
				// point[0] != 'pdf' && 
				if (point && point.length && this[point[0]])
					drawHelper[point[0]](self, self.context, point[1], point[2]);
				// else warn
			}
		},
		getOptions:function(self, opt) {
			opt = opt || {};
			return [
				opt.lineWidth   || self.lineWidth,
				opt.strokeStyle || self.strokeStyle,
				opt.fillStyle   || self.fillStyle,
				opt.globalAlpha || self.globalAlpha,
				opt.globalCompositeOperation || self.globalCompositeOperation,
				opt.lineCap     || self.lineCap,
				opt.lineJoin    || self.lineJoin,
				opt.font        || self.font
			];
		},
		handleOptions:function(self, c, opt, isNoFillStroke) {
			opt = opt || this.getOptions(self);
			c.globalAlpha = opt[3];
			c.globalCompositeOperation = opt[4];
			c.lineCap     = opt[5];
			c.lineJoin    = opt[6];
			c.lineWidth   = opt[0];
			c.strokeStyle = opt[1];
			c.fillStyle   = opt[2];
			c.font        = opt[7];
			if (!isNoFillStroke) {
				c.stroke();
				c.fill();
			}
		},
		line:function(self, c, point, options) {
			c.beginPath();
			c.moveTo(point[0], point[1]);
			c.lineTo(point[2], point[3]);
			drawHelper.handleOptions(self, c, options);
		},
		pencil:function(self, c, point, options) {
			c.beginPath();
			c.moveTo(point[0], point[1]);
			c.lineTo(point[2], point[3]);
			drawHelper.handleOptions(self, c, options);
		},
		marker:function(self, c, point, options) {
			c.beginPath();
			c.moveTo(point[0], point[1]);
			c.lineTo(point[2], point[3]);
			drawHelper.handleOptions(self, c, options);
		},
		arrow:function(self, c, point, options) {
			var mx        = point[0],
				my        = point[1],
				lx        = point[2],
				ly        = point[3],
				arrowSize = arrowHandler.arrowSize,
				angle     = Math.atan2(ly - my, lx - mx);

			if (arrowSize == 10)
				arrowSize = (options ? options[0] : lineWidth) * 5;

			c.beginPath();
			c.moveTo(mx, my);
			c.lineTo(lx, ly);
			drawHelper.handleOptions(self, c, options);
			c.beginPath();
			c.moveTo(lx, ly);
			c.lineTo(lx - arrowSize * Math.cos(angle - Math.PI / 7), ly - arrowSize * Math.sin(angle - Math.PI / 7));
			c.lineTo(lx - arrowSize * Math.cos(angle + Math.PI / 7), ly - arrowSize * Math.sin(angle + Math.PI / 7));
			c.lineTo(lx, ly);
			c.lineTo(lx - arrowSize * Math.cos(angle - Math.PI / 7), ly - arrowSize * Math.sin(angle - Math.PI / 7));
			drawHelper.handleOptions(self, c, options);
		},
		text:function(self, c, point, options) {
			drawHelper.handleOptions(self, c, options);
			c.fillStyle = textHandler.getFillColor(options[2]);
			c.fillText(point[0].substr(1, point[0].length - 2), point[1], point[2]);
		},
		arc:function(self, c, point, options) {
			c.beginPath();
			c.arc(point[0], point[1], point[2], point[3], 0, point[4]);
			drawHelper.handleOptions(self, c, options);
		},
		rect:function(self, c, point, options) {
			drawHelper.handleOptions(self, c, options, true);
			c.strokeRect(point[0], point[1], point[2], point[3]);
			c.fillRect  (point[0], point[1], point[2], point[3]);
		},
		image:function(self, c, point, options) {
			drawHelper.handleOptions(self, c, options, true);
			var image = imageHandler.images[point[5]];
			if (!image) {
				var image     = new Image();
				image.onload  = function() {
					var index = imageHandler.images.length;
					imageHandler.lastImageURL   = image.src;
					imageHandler.lastImageIndex = index;
					imageHandler.images.push(image);
					c.drawImage(image, point[1], point[2], point[3], point[4]);
				};
				image.src = point[0];
				return;
			}
			c.drawImage(image, point[1], point[2], point[3], point[4]);
		},
		pdf:function(self, c, point, options) {
			drawHelper.handleOptions(self, c, options, true);
			var image = self.pdf.images[point[5]];
			if (!image) {
				var image     = new Image();
				image.onload  = function() {
					var index = self.image.images.length;
					self.pdf.lastPage  = image.src;
					self.pdf.lastIndex = index;
					self.pdf.images.push(image);
					c.drawImage(image, point[1], point[2], point[3], point[4]);
				};
				image.src = point[0];
				return;
			}
			c.drawImage(image, point[1], point[2], point[3], point[4]);
			pdfHandler.reset_pos(self, point[1], point[2]);
		},
		quadratic:function(self, c, point, options) {
			c.beginPath();
			c.moveTo(point[0], point[1]);
			c.quadraticCurveTo(point[2], point[3], point[4], point[5]);
			drawHelper.handleOptions(self, c, options);
		},
		bezier:function(self, c, point, options) {
			c.beginPath();
			c.moveTo(point[0], point[1]);
			c.bezierCurveTo(point[2], point[3], point[4], point[5], point[6], point[7]);
			drawHelper.handleOptions(self, c, options);
		}
	};
	handlers = {
		line:       {mousedown:lineHandler.mousedown,   mouseup:lineHandler.mouseup,   mousemove:lineHandler.mousemove},
		arc:        {mousedown:arcHandler.mousedown,    mouseup:arcHandler.mouseup,    mousemove:arcHandler.mousemove},
		rect:       {mousedown:rectHandler.mousedown,   mouseup:rectHandler.mouseup,   mousemove:rectHandler.mousemove},
		quad:       {mousedown:quadHandler.mousedown,   mouseup:quadHandler.mouseup,   mousemove:quadHandler.mousemove},
		bezier:     {mousedown:bezierHandler.mousedown, mouseup:bezierHandler.mouseup, mousemove:bezierHandler.mousemove},
		dragLast:   {mousedown:dragHelper.mousedown,    mouseup:dragHelper.mouseup,    mousemove:dragHelper.mousemove},
		pencil:     {mousedown:pencilHandler.mousedown, mouseup:pencilHandler.mouseup, mousemove:pencilHandler.mousemove},
		marker:     {mousedown:markerHandler.mousedown, mouseup:markerHandler.mouseup, mousemove:markerHandler.mousemove},
		eraser:     {mousedown:eraserHandler.mousedown, mouseup:eraserHandler.mouseup, mousemove:eraserHandler.mousemove},
		dragAll:    {mousedown:dragHelper.mousedown,    mouseup:dragHelper.mouseup,    mousemove:dragHelper.mousemove},
		text:       {mousedown:textHandler.mousedown,   mouseup:textHandler.mouseup,   mousemove:textHandler.mousemove},
		image:      {mousedown:imageHandler.mousedown,  mouseup:imageHandler.mouseup,  mousemove:imageHandler.mousemove},
		arrow:      {mousedown:arrowHandler.mousedown,  mouseup:arrowHandler.mouseup,  mousemove:arrowHandler.mousemove},
		pdf:        {mousedown:pdfHandler.mousedown,	mouseup:pdfHandler.mouseup,    mousemove:pdfHandler.mousemove}
	};

	return CanvasDesigner
});
