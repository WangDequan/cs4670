/*
	krpano textfield plugin
	1.0.8.15
*/

package
{
	import flash.display.*;
	import flash.events.*;
	import flash.system.*;
	import flash.net.*;
	import flash.filters.*;
	import flash.geom.*;
	import flash.text.*;
	import flash.utils.*;
	import flash.xml.*;
	import flash.ui.Mouse;

	import krpano_as3_interface;


	[SWF(width="400", height="300", backgroundColor="#000000")]
	public class textfield extends Sprite
	{
		// krpano as3 interface
		private var krpano:krpano_as3_interface = null;

		public var pluginpath : String = null;
		public var pluginobj  : Object = null;

		public var bg  : Shape     = null;
		public var txt : TextField = null;

		public var txt_width  : int = 400;
		public var txt_height : int = 300;



		public function textfield()
		{
			if (stage == null)
			{
				this.addEventListener(Event.ADDED_TO_STAGE, startplugin);
				this.addEventListener(Event.UNLOAD,         stopplugin);
			}
			else
			{
				// direct startup - show version info
				stage.scaleMode = StageScaleMode.NO_SCALE;
				stage.align     = StageAlign.TOP_LEFT;

				var txt:TextField = new TextField();
				txt.textColor = 0xFFFFFF;
				txt.selectable = false;

				txt.htmlText =	"krpano " + "1.0.8.15" + "\n\n" +
								"<b>textfield plugin</b>"  + "\n\n" +
								"(build " + "CUSTOM" + ")";

				var f:TextFormat = new TextFormat();
				f.font = "_sans";
				f.size = 14;
				txt.autoSize = f.align = "center";
				txt.setTextFormat(f);

				addChild(txt);

				var resizefu:Function = function(event:Event):void
				{
					txt.x = (stage.stageWidth  - txt.width)/2;
					txt.y = (stage.stageHeight - txt.height)/2;
				}

				stage.addEventListener(Event.RESIZE, resizefu);

				resizefu(null);
			}
		}


		private function startplugin(evt:Event):void
		{
			this.removeEventListener(Event.ADDED_TO_STAGE, startplugin);

			if (krpano == null)
			{
				// get krpano interface
				krpano = krpano_as3_interface.getInstance();

				if ( krpano.get("version") < "1.0.8" )
				{
					krpano.call("error(textfield plugin - wrong krpano version - min. 1.0.8 needed);");
					return;
				}

				// add krpano plugin listeners

				// register event to get the krpano name of the plugin
				krpano.addPluginEventListener(this, krpano_as3_interface.PLUGINEVENT_REGISTER, registerEvent);

				// resize event to set the size of the textfield
				krpano.addPluginEventListener(this, krpano_as3_interface.PLUGINEVENT_RESIZE,   resizeEvent);
				krpano.addPluginEventListener(this, krpano_as3_interface.PLUGINEVENT_UPDATE,   updateEvent);
			}
		}



		private function stopplugin(evt:Event):void
		{
			// remove textfield link event listener
			txt.removeEventListener(TextEvent.LINK, link_event);

			// remove krpano event listeners
			krpano.removePluginEventListener(this, krpano_as3_interface.PLUGINEVENT_REGISTER, registerEvent);
			krpano.removePluginEventListener(this, krpano_as3_interface.PLUGINEVENT_RESIZE,   resizeEvent);
			krpano.removePluginEventListener(this, krpano_as3_interface.PLUGINEVENT_UPDATE,   updateEvent);

			// remove all elements
			removeChild(bg);
			bg = null;

			removeChild(txt);
			txt = null;

			krpano = null;
		}



		private function registerEvent(evt:DataEvent):void
		{
			// register event - "evt.data" is the krpano xml name/path of the plugin (e.g. "plugin[txt1]" or "hotspot[textspot1]")

			pluginpath = evt.data;
			pluginobj  = krpano.get(pluginpath);		// get the krpano plugin object

			// register custom attributes with their default value (note - only lowercase attributes are possible!)
			pluginobj.registerattribute("html",            "");
			pluginobj.registerattribute("css",             "");
			pluginobj.registerattribute("autosize",        "none");
			pluginobj.registerattribute("autoheight",      String(pluginobj.autosize).toLowerCase() != "none" , function(b:Boolean):void{ pluginobj.autosize = b ? "center" : "none"; }, function():Boolean{ return String(pluginobj.autosize).toLowerCase() == "none" ? false : true });
			pluginobj.registerattribute("vcenter",         false);
			pluginobj.registerattribute("wordwrap",        true);
			pluginobj.registerattribute("background",      true);
			pluginobj.registerattribute("backgroundcolor", 0xFFFFFF);
			pluginobj.registerattribute("backgroundalpha", 1.0);
			pluginobj.registerattribute("border",          false);
			pluginobj.registerattribute("bordercolor",     0x000000);
			pluginobj.registerattribute("borderwidth",     1);
			pluginobj.registerattribute("borderalpha",     1.0);
			pluginobj.registerattribute("roundedge",       0);
			pluginobj.registerattribute("selectable",      true);
			pluginobj.registerattribute("glow",            0);
			pluginobj.registerattribute("glowcolor",       0xFFFFFF);
			pluginobj.registerattribute("blur",            0);

			pluginobj.registerattribute("shadow",          0);
			pluginobj.registerattribute("shadowrange",     4);
			pluginobj.registerattribute("shadowangle",     45);
			pluginobj.registerattribute("shadowcolor",     0x000000);
			pluginobj.registerattribute("shadowalpha",     1.0);

			pluginobj.registerattribute("textglow",        0);
			pluginobj.registerattribute("textglowcolor",   0xFFFFFF);
			pluginobj.registerattribute("textblur",        0);

			pluginobj.registerattribute("textshadow",      0);
			pluginobj.registerattribute("textshadowrange", 4);
			pluginobj.registerattribute("textshadowangle", 45);
			pluginobj.registerattribute("textshadowcolor", 0x000000);
			pluginobj.registerattribute("textshadowalpha", 1.0);

			// add custom functions
			pluginobj.update = updateHTML;


			// create a background shape for the textfield
			bg = new Shape();

			// create the as3 textfield itself
			txt = new TextField();
			txt.htmlText      = "";
			txt.multiline     = true;
			txt.wordWrap      = true;
			txt.border        = false;
			txt.background    = false;
			txt.condenseWhite = true;
			txt.width         = txt_width;
			txt.height        = txt_height;

			// textfield link event listener
			txt.addEventListener(TextEvent.LINK, link_event);

			// add background and textfield
			this.addChild(bg);
			this.addChild(txt);

			// update the style and content of the textfield
			updateSTYLE();
			updateHTML();
		}



		private function updateEvent(dataevent:DataEvent):void
		{
			// the update event sends the name of the changed attribute in the "dataevent.data" variable

			// do here a quick search for the changed attribute and call the corresponding update function
			var changedattribute:String = "." + String( dataevent.data ) + ".";
			const data_attributes :String = ".html.css.";
			const style_attributes:String = ".autosize.vcenter.wordwrap.background.backgroundcolor.backgroundalpha.border.bordercolor.borderalpha.borderwidth.roundedge.selectable.glow.blur.shadow.textglow.textblur.textshadow.";

			if ( data_attributes.indexOf(changedattribute) >= 0 )
			{
				updateHTML();
			}
			else if ( style_attributes.indexOf(changedattribute) >= 0 )
			{
				updateSTYLE();
			}
		}



		private function resizeEvent(dataevent:DataEvent):void
		{
			var resizesize:String = dataevent.data;		// size has the format WIDTHxHEIGHT

			var width :int;
			var height:int;

			width  = parseInt(resizesize);
			height = parseInt(resizesize.slice(resizesize.indexOf("x")+1));

			// set the size of the textfield
			txt.width  = width;
			txt.height = height;

			// save size
			txt_width  = width;
			txt_height = height;

			pluginobj.imagewidth = width;
			pluginobj.imageheight = height;

			// update background shape
			updateSTYLE();

			if (txt.autoSize != "none" || pluginobj.vcenter == true)
			{
				delayedGetTextHeight();
			}

		}



		private function link_event(textevent:TextEvent):void
		{
			// pass the text after the "event:" link to krpano

			krpano.call( textevent.text, null, pluginobj );
		}


		private function getshadow(prefix:String):DropShadowFilter
		{
			var shadow      : Number = Number( pluginobj[prefix+"shadow"]      );
			var shadowrange : Number = Number( pluginobj[prefix+"shadowrange"] );
			var shadowangle : Number = Number( pluginobj[prefix+"shadowangle"] );
			var shadowcolor : int    =    int( pluginobj[prefix+"shadowcolor"] );
			var shadowalpha : Number = Number( pluginobj[prefix+"shadowalpha"] );

			return new DropShadowFilter(shadow, shadowangle, shadowcolor, shadowalpha, shadowrange, shadowrange);
		}


		private function updateSTYLE():void
		{
			// pass the krpano parameters to the as3 textfield
			switch ( String( pluginobj.autosize ).toLowerCase() )
			{
				case "true":
				case "auto":
				case "left":  	txt.autoSize = "left";
								break;

				case "center":	txt.autoSize = "center";
								break;

				case "right": 	txt.autoSize = "right";
								break;

				case "none":
				default:      	txt.autoSize = pluginobj.vcenter ? "left" : "none";
								break;
			}


			txt.wordWrap   = pluginobj.wordwrap;
			txt.selectable = pluginobj.selectable;

			// better text quality inside browser:
			txt.antiAliasType = AntiAliasType.ADVANCED;
			txt.gridFitType = GridFitType.SUBPIXEL;

			// update/draw the background shape
			bg.graphics.clear();

			// draw a background and/or border?
			if (pluginobj.background || pluginobj.border)
			{
				var off:Number = 0;

				var bgwidth:int = txt_width;
				var bgheight:int = txt_height;

				if (txt.autoSize == "none" || pluginobj.vcenter == true)
				{
					bgwidth = int(pluginobj.imagewidth);
					bgheight = int(pluginobj.imageheight);
				}

				off = pluginobj.border ? pluginobj.borderwidth : 0;
				if (off < 0)
					off = 0;

				bg.alpha = pluginobj.backgroundalpha;

				// draw background
				if (pluginobj.background)
					bg.graphics.beginFill(pluginobj.backgroundcolor);

				if (pluginobj.roundedge <= 0)
					bg.graphics.drawRect(-off,-off,bgwidth+2*off,bgheight+2*off);
				else
					bg.graphics.drawRoundRect(-off,-off,bgwidth+2*off,bgheight+2*off, pluginobj.roundedge+off);

				// draw border
				if (pluginobj.border && pluginobj.borderwidth > 0)
				{
					bg.graphics.beginFill(pluginobj.bordercolor, pluginobj.borderalpha);

					if (pluginobj.roundedge <= 0)
					{
						bg.graphics.drawRect(-off,-off,bgwidth+off*2,bgheight+off*2);
						bg.graphics.drawRect(0,0,bgwidth,bgheight);
					}
					else
					{
						bg.graphics.drawRoundRect(-off,-off,bgwidth+off*2,bgheight+off*2, pluginobj.roundedge+off);
						bg.graphics.drawRoundRect(0,0,bgwidth,bgheight, pluginobj.roundedge-off);
					}

					bg.graphics.endFill();
				}
			}


			// create and apply filters for the background shape
			var filters:Array = new Array();

			if (pluginobj.glow > 0)
			{
				filters.push( new GlowFilter(int(pluginobj.glowcolor), 1.0, Number(pluginobj.glow),Number(pluginobj.glow)) );
			}

			if (pluginobj.blur > 0)
			{
				// blur = blur range
				filters.push( new BlurFilter(pluginobj.blur, pluginobj.blur) );
			}

			if (pluginobj.shadow > 0)
			{
				filters.push( getshadow("") );
			}

			// set or remove the filters
			bg.filters = filters.length > 0 ? filters : null


			// create and apply filters for the text itself
			var textfilters:Array = new Array();

			if (pluginobj.textglow > 0)
			{
				textfilters.push( new GlowFilter(pluginobj.textglowcolor, 1.0, pluginobj.textglow,pluginobj.textglow) );
			}

			if (pluginobj.textblur > 0)
			{
				// textblur = blur range
				textfilters.push( new BlurFilter(pluginobj.textblur,  pluginobj.textblur) );
			}

			if (pluginobj.textshadow > 0)
			{
				textfilters.push( getshadow("text") );
			}

			// set or remove the filters
			txt.filters = textfilters.length > 0 ? textfilters : null;
		}


		private function updateHTML():void
		{
			var css:StyleSheet = new StyleSheet();

			var cssdata :String = pluginobj.css;
			var htmldata:String = pluginobj.html;
			var css_p_fix:Boolean = false;

			if (htmldata == null)
				htmldata = "";

			if (cssdata == null || cssdata == "")
			{
				txt.styleSheet = null;
			}
			else
			{
				if (cssdata.indexOf("data:") == 0 )
				{
					// load the content of a <data> tag
					cssdata = krpano.get("data[" + cssdata.slice(5) + "].content");
					if (cssdata == null)
						cssdata = "";
				}
				else
				{
					// directly use the given css
					cssdata = unescape(cssdata);
				}

				if (cssdata && (cssdata.indexOf("{") < 0))
				{
					// no tags -> put all inside p{}
					cssdata = "p{" + cssdata + "}";
					css_p_fix = true;
				}

				css.parseCSS( cssdata );
				txt.styleSheet = css;
			}

			if (htmldata.indexOf("data:") == 0 )
			{
				// load the content of a <data> tag
				htmldata = krpano.get("data[" + htmldata.slice(5) + "].content");
				if (htmldata == null)
					htmldata = "";
			}

			// (<> chars are not possible in a xml attribure, therefore provide the usage of [] instead, use '[[' and ']]' to get a '[' or ']')
			htmldata = htmldata.split("[").join("<").split("<<").join("[").split("]").join(">").split(">>").join("]");

			htmldata = unescape(htmldata);

			if (css_p_fix && htmldata && htmldata.indexOf("<p>") < 0)
			{
				htmldata = "<p>" + htmldata + "</p>";
			}

			if (htmldata == null)
			{
				htmldata = "";
			}

			// fix textfield &nbsp; bug
			htmldata = htmldata.split(String.fromCharCode(0xFFA0)).join(String.fromCharCode(0x00A0));

			txt.htmlText = htmldata;

			if (txt.autoSize != "none" && pluginobj.vcenter == false)
			{
				// the as3 textfield autosizing is used
				txt_height = txt.height;

				updateSIZE();
				delayedGetTextHeight();
			}
			else if (pluginobj.vcenter == true)
			{
				txt.visible = false;

				updateSIZE();
				delayedGetTextHeight();
			}

			// save the current text only height also to the xml as "textheight" variable
			pluginobj.textheight = txt_height;
		}


		private function delayedGetTextHeight():void
		{
			// the textfield unfortuntaly doesn't provide the right size immediately
			// therefore add a short delay to get the real size
			var updatetimer:Timer = new Timer(0.001,1);
			updatetimer.addEventListener("timer", updateSIZE);
			updatetimer.start();
		}


		private function updateSIZE(te:TimerEvent=null):void
		{
			if (!krpano || !pluginobj)
				return;

			txt_height = txt.height;

			var size_has_changed:Boolean = false;

			// update the real textfield and text sizes in krpano
			if (txt.autoSize != "none" && pluginobj.vcenter == false)
			{
				if (pluginobj.height != txt_height)
				{
					pluginobj.height = txt_height;
					size_has_changed = true;
				}

				pluginobj.textheight = txt.textHeight;
				pluginobj.imageheight = height;
			}

			if (pluginobj.vcenter == true)
			{
				txt.y = pluginobj.pixelheight/2 - txt_height/2;

				if (te != null)
				{
					txt.visible = true;
				}
			}
			else
			{
				txt.y = 0;
			}

			if ( size_has_changed && pluginobj.onautosized )
			{
				pluginobj.updatepos();

				krpano.call( pluginobj.onautosized, null, pluginobj, true );
			}


			// update the background shape
			updateSTYLE();
		}
	}
}
