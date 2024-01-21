function tracker_onclick(e)
{
	WS.send("tracker add git " + $(".tracker-input", e.target.parentNode).val())
	debugger;
}
