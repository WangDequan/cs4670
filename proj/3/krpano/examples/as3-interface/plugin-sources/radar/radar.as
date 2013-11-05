/*
	krpano radar plugin
	1.0.8.15
*/

package
{
	import flash.display.*;
	import flash.text.*;
	import flash.events.*;
	import flash.utils.*;
	import flash.system.*;

	import krpano_as3_interface;


	[SWF(width="256", height="256", backgroundColor="#000000")]
	public class radar extends Sprite
	{
		// krpano as3 interface
		public var krpano:krpano_as3_interface = null;

		// krpano objects
		public var viewobject   : Object = null;
		public var pluginobject : Object = null;

		// style settings
		static private const radarradius : int = 128;		// half of the SWF width

		// internal sprite and update timer
		private var radarsprite : Sprite = null;
		private var updatetimer : Timer  = null;

		// local getter/setter variables
		private var heading       : Number = 0.0;
		private var headingoffset : Number = 90.0;
		private var invert        : Boolean = false;
		private var fillcolor     : uint = 0xFFFFFF;
		private var linecolor     : uint = 0xFFFFFF;
		private var fillalpha     : Number = 0.5;
		private var linealpha     : Number = 0.3;
		private var linewidth     : Number = 0.0;

		// internals
		private var needupdate    : Boolean = true;


		public function radar()
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
								"<b>radar plugin</b>" +
								"\n\n(build " + "CUSTOM" + ")";

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



		// start of plugin

		private function startplugin(evt:Event):void
		{
			this.removeEventListener(Event.ADDED_TO_STAGE, startplugin);

			if (krpano == null)
			{
				// setup krpano interface
				krpano = krpano_as3_interface.getInstance();

				if ( krpano.get("version") < "1.0.8.14" || krpano.get("build") < "2011-04-04" )
				{
					krpano.trace(krpano_as3_interface.ERROR, "radar plugin - too old krpano version (min. 1.0.8.14)");
					return;
				}

				krpano.addPluginEventListener(this, krpano_as3_interface.PLUGINEVENT_REGISTER, registerEvent);
			}
		}



		private function registerEvent(evt:DataEvent):void
		{
			// get the radar plugin object
			pluginobject = krpano.get(evt.data);		// evt.data is the name/path of the plugin, e.g. "plugin[radar]"

			// get the krpano view object
			viewobject = krpano.get("view");

			// radar sprite
			radarsprite = new Sprite();
			radarsprite.tabEnabled = false;
			radarsprite.buttonMode = true;		// hand cursor
			radarsprite.addEventListener(MouseEvent.MOUSE_DOWN, mouse_down);

			// register attributes with their type and default value
			pluginobject.registerattribute("heading",       0.0,      function(v:*):void{ heading=Number(v); needupdate=true; }, function():Number{ return heading; });
			pluginobject.registerattribute("headingoffset", 90.0,     function(v:*):void{ headingoffset=Number(v); needupdate=true; }, function():Number{ return headingoffset; });
			pluginobject.registerattribute("invert",        false,    function(v:*):void{ invert=String(v).toLowerCase()=="true"; needupdate=true; }, function():Boolean{ return invert; });
			pluginobject.registerattribute("fillcolor",     0xFFFFFF, function(v:*):void{ fillcolor=uint(v); needupdate=true; }, function():uint{ return fillcolor; });
			pluginobject.registerattribute("linecolor",     0xFFFFFF, function(v:*):void{ linecolor=uint(v); needupdate=true; }, function():uint{ return linecolor; });
			pluginobject.registerattribute("fillalpha",     0.5,      function(v:*):void{ fillalpha=Number(v); needupdate=true; }, function():Number{ return fillalpha; });
			pluginobject.registerattribute("linealpha",     0.3,      function(v:*):void{ linealpha=Number(v); needupdate=true; }, function():Number{ return linealpha; });
			pluginobject.registerattribute("linewidth",     0.0,      function(v:*):void{ linewidth=Number(v); needupdate=true; }, function():Number{ return linewidth; });

			addChild(radarsprite);

			updatetimer = new Timer(1000.0/30.0, 0);	// updaterate: 30fps
			updatetimer.addEventListener(TimerEvent.TIMER, radarHandler);

			updatetimer.start();
		}



		private function stopplugin(evt:Event):void
		{
			if (updatetimer)
			{
				updatetimer.stop();
				updatetimer = null;
			}

			radarsprite.removeEventListener (MouseEvent.MOUSE_DOWN, mouse_down);
			removeChild(radarsprite);

			krpano.removePluginEventListener(this, krpano_as3_interface.PLUGINEVENT_REGISTER, registerEvent);

			pluginobject = null;
			radarsprite  = null;
			viewobject   = null;
		}



		private function mouse_down(evt:MouseEvent):void
		{
			if (evt)
				evt.stopPropagation();
				
			startdrag = true;

			mouse_move(evt);

			stage.addEventListener(MouseEvent.MOUSE_MOVE, mouse_move);
			stage.addEventListener(MouseEvent.MOUSE_UP,   mouse_up);
		}


		private function mouse_up(evt:MouseEvent):void
		{
			mouse_move(evt);

			stage.removeEventListener(MouseEvent.MOUSE_MOVE, mouse_move);
			stage.removeEventListener(MouseEvent.MOUSE_UP,   mouse_up);
		}



		private var startdrag:Boolean = false;
		private var startangle:Number = 0;


		private function mouse_move(evt:MouseEvent):void
		{
			var dx:Number;
			var dy:Number;
			var angle:Number;

			var r:Number = radarradius;

			dx = this.mouseX - r;
			dy = this.mouseY - r;

			angle = (Math.atan2(dy,dx) * 180.0 / Math.PI);

			if (isNaN(heading) )
				heading = 0;

			angle = angle - heading;

			if (startdrag == true)
			{
				startangle = angle - Number( viewobject.hlookat );
				startdrag = false;
			}
			else
			{
				viewobject.hlookat = (angle - startangle);
			}
		}



		private var last_hlookat:Number = 0;
		private var last_fov:Number = 0;


		private function radarHandler(e:TimerEvent):void
		{
			var hlookat:Number   = Number( viewobject.hlookat );
			var fov:Number       = Number( viewobject.hfov    );

			hlookat += heading + headingoffset - 90;

			if (invert)
			{
				fov = 360 - fov;
				hlookat += 180.0;
			}

			if (Math.abs(hlookat-last_hlookat) > 1 || Math.abs(fov-last_fov) > 2)
			{
				last_hlookat = hlookat;
				last_fov     = fov;

				needupdate = true;
			}

			if (needupdate)
			{
				var r:Number = radarradius;

				var g:Graphics = radarsprite.graphics;
				g.clear();

				g.beginFill(fillcolor, fillalpha);
				g.lineStyle(linewidth, linecolor, linealpha);

				g.moveTo(r,r);

				var steps:int = int( 1 + fov / 10 );
				if (steps < 2)
					steps = 2;

				var i:int = 0;

				var a:Number;
				var px:Number;
				var py:Number;

				for (i=0; i<steps; i++)
				{
					a = ((i==(steps-1)) ? (hlookat+fov/2) : (hlookat - fov/2 + i * fov / steps)) * Math.PI / 180.0;

					px = r + r * Math.cos(a);
					py = r + r * Math.sin(a);

					g.lineTo(px,py);
				}

				g.endFill();
			}
		}
	}
}
