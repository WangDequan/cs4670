/*
	krpano
	follow mouse plugin
	1.0.8.14
*/

package
{
	import flash.display.*;
	import flash.text.*;
	import flash.events.*;
	import flash.utils.*;
	import flash.system.*;


	[SWF(width="400", height="300", backgroundColor="#000000")]
	public class followmouse extends Sprite
	{
		private var krpano : Object = null;

		private var control_mousetype_backup : String = null;


		public function followmouse()
		{
			if (stage == null)
			{
				// startup when loaded inside krpano
				this.addEventListener(Event.ADDED_TO_STAGE, versioncheck);
			}
			else
			{
				// direct startup - show plugin version info
				stage.scaleMode = "noScale";
				stage.align = "TL";

				var txt:TextField = new TextField();
				txt.defaultTextFormat = new TextFormat("_sans",14,0xFFFFFF,false,false,false,null,null,"center");
				txt.autoSize = "center";
				txt.htmlText = "krpano\n\nfollowmouse plugin";
				addChild(txt);

				var resizefu:Function = function(event:Event=null):void
				{
					txt.x = (stage.stageWidth  - txt.width )/2;
					txt.y = (stage.stageHeight - txt.height)/2;
				}

				stage.addEventListener(Event.RESIZE, resizefu);

				resizefu();
			}
		}


		private function versioncheck(evt:Event):void
		{
			// compatibility check of the krpano version by using the old plugin interface:
			// - the "version" must be at least "1.0.8.14"
			// - and the "build" must be "2011-05-10" or greater
			this.removeEventListener(Event.ADDED_TO_STAGE, versioncheck);

			var oldkrpanointerface:Object = (getDefinitionByName("krpano_as3_interface") as Class)["getInstance"]();

			if (oldkrpanointerface.get("version") < "1.0.8.14" || oldkrpanointerface.get("build") < "2011-05-10")
			{
				oldkrpanointerface.trace(3, "followmouse plugin - too old krpano viewer version (min. 1.0.8.14)");
			}
		}



		// registerplugin
		// - the start for the plugin
		// - this function will be called from krpano when the plugin will be loaded
		public function registerplugin(krpanointerface:Object, pluginfullpath:String, pluginobject:Object):void
		{
			krpano = krpanointerface;

			stage.addEventListener(Event.MOUSE_LEAVE,     mouse_out);
			stage.addEventListener(MouseEvent.MOUSE_MOVE, mouse_move);
		}


		// unloadplugin
		// - the end for the plugin
		// - this function will be called from krpano when the plugin will be removed
		public function unloadplugin():void
		{
			stage.removeEventListener(Event.MOUSE_LEAVE,     mouse_out);
			stage.removeEventListener(MouseEvent.MOUSE_MOVE, mouse_move);
		}


		private function mouse_move (event:MouseEvent):void
		{
			var mx:Number = stage.mouseX;
			var my:Number = stage.mouseY;
			var sx:Number = stage.stageWidth  * 0.5;
			var sy:Number = stage.stageHeight * 0.5;


			// calc motion vectors: -1.0 to +1.0
			var vx:Number = (mx - sx) / sx;
			var vy:Number = (my - sy) / sy;

			if ( event.buttonDown || (vx > -0.5 && vx < 0.5 && vy > -0.5 && vy < 0.5) )
			{
				// middle area - normal control

				krpano.hlookat_moveforce = 0;
				krpano.vlookat_moveforce = 0;
			}
			else
			{
				// outer area - automatic followmouse movement

				vx = 2.0 * (vx < 0 ? -1.0 : +1.0) * (Math.max(Math.abs(vx),0.5) - 0.5);
				vy = 2.0 * (vy < 0 ? -1.0 : +1.0) * (Math.max(Math.abs(vy),0.5) - 0.5);

				// stop very slow moving
				if (Math.abs(vx) < 0.01)	vx = 0;
				if (Math.abs(vy) < 0.01)	vy = 0;

				// set move forces
				krpano.hlookat_moveforce = vx;
				krpano.vlookat_moveforce = vy;
			}
		}


		private function mouse_out(event:*):void
		{
			krpano.hlookat_moveforce = 0;
			krpano.vlookat_moveforce = 0;
		}

	}
}
