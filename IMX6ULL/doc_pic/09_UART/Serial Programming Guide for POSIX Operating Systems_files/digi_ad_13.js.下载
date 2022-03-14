function dglr_RS_CHK9() {
var a_t=0, a_f=0;
var m_f_m=.40;
var x=0,y=0;
var p=this.window;
var pp=p.parent, v;
while (pp!=p) {
	if ((document.getElementById)&&(!document.all)) { eval("try {v=pp.innerWidth} catch (v) {pp=p}") } 
	else { if (typeof(pp.document)!="object") pp=p } 
	if (pp!=p) {p=pp; pp=p.parent}
}
if (p.innerWidth) {
	a_t=p.innerWidth*p.innerHeight;
	a_f=window.innerWidth*window.innerHeight;
} else if (p.document) if (p.document.body) {
	if (p.document.body.offsetWidth) {
		a_t=p.document.body.offsetWidth*p.document.body.offsetHeight;
		if (window.document.body) {
			a_f=window.document.body.offsetWidth*window.document.body.offsetHeight;
		}
	} else return 1;
} else return 1;
if (!a_t) return 1;
var h_f=a_f/a_t;
if (h_f>m_f_m) return 1;
return 0;
}

if (!window.top) rs_DLR=1;
else if (!window.top.frames) rs_DLR=1;
else if ((window==window.top)&&(window.top.frames.length==0)) rs_DLR=1;
else if ((window!=window.top)||(window.top.frames.length>0)) {
	rs_DLR=dglr_RS_CHK9();
	if (document.body) if (document.body.innerHTML) {
		var d=document.body.innerHTML.toLowerCase();
		if (window.frames.length>=1) {
			if ((rs_DLR==1)&&(d.indexOf("<iframe")>=0)) rs_DLR=1;
			else rs_DLR=0;
			}
	}
} else rs_DLR=1;

if (window.location.pathname.indexOf("/_ad/")==0) rs_DLR=0;

if ((window.location.pathname.indexOf("/index")==0)||(window.location.pathname=="/")) rs_DLR=1;

if (window.location.pathname.indexOf("/_ppricerca/")==0) { rs_DLR=0; rs_DLRERR=1; }

for (n=0; n<document.images.length; n++) if (document.images[n].src) if ( (document.images[n].src.indexOf("//secure-it.imrworldwide.com/cgi-bin/m?")!=-1 )  || (document.images[n].src.indexOf("//b.scorecardresearch.com/b?") !=-1 ) ) rs_DLR=0;
