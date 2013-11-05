/*
	krpano embedding into Flash CS example
*/

var krpano:Object = null;


function krpano_load():void
{
	var krpanoloader = new Loader();
	krpanoloader.contentLoaderInfo.addEventListener(Event.COMPLETE, krpano_load_complete); 
	krpanoloader.load(new URLRequest("krpano.swf"));
}

function krpano_load_complete(event:Event):void
{
	(event.target.content as Object).embeddedstartup(stage, krpano_ready);
}


// start
krpano_load();


function krpano_ready(krpanointerface:Object):void
{
	krpano = krpanointerface;
	
	// open the krpano log and show a message:
	krpano.call("showlog(true);");
	krpano.set("debugmode", true); // show 0/debug traces 
	krpano.trace(0,"ready...");
	

	// load a inline xml with a preview image as demo
	krpano.call("loadxml('<preview type=\"grid(cube);\" />');");
	
	// example: load a pano xml:
	//krpano.call("loadpano(pano.xml,null,MERGE,BLEND(1));");
	
	
	// get notified about krpano resize events
	krpano.set("events.onresize", krpano_resize_event);
	
	
	// change the pano area to a fixed size:
	var area:Object = krpano.get("area");
	area.x      = 20;
	area.y      = 20;
	area.width  = 400;
	area.height = 300;
	
		
	// examples how to add/change plugins from as3:
	
	// 1. the "set-interface" way:
	krpano.call("addplugin(button1);");
	krpano.set("plugin[button1].url", "button.jpg");
	krpano.set("plugin[button1].parent", "STAGE");
	krpano.set("plugin[button1].align", "righttop");
	krpano.set("plugin[button1].x", 10);
	krpano.set("plugin[button1].y", 10);
	krpano.set("plugin[button1].onhover", "showtext(toggle pano layer);");
	krpano.set("plugin[button1].onclick", toggle_krpano_visibilty);
	
	// 2. the direct object way (faster)
	krpano.call("addplugin(button2);");
	var button2:Object = krpano.get("plugin[button2]");
	button2.url     = "button.jpg";
	button2.parent  = "STAGE";
	button2.align   = "righttop";
	button2.x       = 10;
	button2.y       = 50;
	button2.onhover = "showtext(change area size);";
	button2.onclick = toggle_krpano_area;
}


function krpano_resize_event():void
{
	var area:Object = krpano.get("area");
	
	krpano.trace(0,"pano resize - pos=" + area.pixelx + "," + area.pixely + " size=" + area.pixelwidth + "x" + area.pixelheight);
}


function toggle_krpano_visibilty():void
{
	var imagelayer:Sprite = krpano.get("image.layer");
	
	if (imagelayer.visible == true)
		imagelayer.visible = false;
	else
		imagelayer.visible = true;
		
	// toggle also the plugin layer
	krpano.set("plugin.visible", imagelayer.visible);
}

function toggle_krpano_area():void
{
	var area:Object = krpano.get("area");
	
	// just toggle between two sizes
	if (area.x == 20)
	{
		area.x      = 0;
		area.y      = 0;
		area.width  = "100%";
		area.height = "100%";
	}
	else
	{
		area.x      = 20;
		area.y      = 20;
		area.width  = 400;
		area.height = 300;
	}

}

