if ( typeof(Libero) == 'undefined' ) {
	Libero = {
		Version: '1.1.2',
		Browser: (function(){
			var ua = navigator.userAgent;
			var isOpera = Object.prototype.toString.call(window.opera) == '[object Opera]';
			return {
				IE:             !!window.attachEvent && !isOpera,
				Opera:          isOpera,
				WebKit:         ua.indexOf('AppleWebKit/') > -1,
				Gecko:          ua.indexOf('Gecko') > -1 && ua.indexOf('KHTML') === -1,
				MobileSafari:   /Apple.*Mobile/.test(ua),
				isMobile: /iphone|ipod|android|blackberry|opera|mini|windows\s*(ce|phone)|palm|smartphone|iemobile/i.test(ua.toLowerCase()),
				isTablet: /ipad|android|xoom|sch-i800|playbook|tablet|kindle/i.test(ua.toLowerCase())
			}
		})()
    };
}
  
Libero.Stash = {};

// test prototype library
if ( typeof(Prototype) == 'undefined' ) {
	var Class = {
		create: function() {
			return function() {
				this.initialize.apply(this, arguments);
			}
		}
	};
	Object.extend = function(destination, source) {
		for (var property in source) {
			destination[property] = source[property];
  		}
		return destination;
	};
}

Libero.objectExtend = function(destination, source) {
	for (var property in source) {
		if ( typeof(source[property]) != 'undefined' ) {
			destination[property] = source[property]
		}
	}
	return destination;
};

// Libero.HF: header footer class utilities  
Libero.HF = Class.create();

Libero.HF.show = function (id,mode) {
	if (document.getElementById) {
		if ( typeof(mode) == 'undefined' )
			mode = 'block'; 
		var el = document.getElementById(id);
		if (el) {
			el.style.display = mode;
		}
	}
}

Libero.HF.hide = function (id) {
	if (document.getElementById) {
		var el = document.getElementById(id);
		if (el) {
			el.style.display = 'none';
		}
	}
}
Libero.HF.toggle = function(id,mode) {
	if (document.getElementById) {
		if ( typeof(mode) == 'undefined' )
			mode = 'block'; 
		var el = document.getElementById(id);
		if (el) {
			if (el.style.display == mode) {
				el.style.display = 'none';
			}
			else {
				el.style.display = mode;
			}
		}
	}
}

Libero.HF.ariannaSearch = function (url, empty_url, cksrv_a, cksrv_b, cksrv_c, q, encode, target) {
	encode = encode == undefined ? 1 : encode;
	target = target == undefined ? null : target;
	// empty query
	if ( q.match(/^\s*$/) ) {
		if ( typeof(empty_url) != 'undefined' ) {
			ckSrv(empty_url,cksrv_a,cksrv_b,cksrv_c,target);
			return false; 		
		}
		else 
			return false;
	}
	else {
		ckSrv(url + ( encode ? encodeURIComponent(q) : q ),cksrv_a,cksrv_b,cksrv_c,target);
		return false; 
	}
}

// Adv
function adv_flashDisp (flashobj) {
	if (typeof(flashobj)!='undefined'&&flashobj!='') {
		document.write(flashobj);
	}
}

Libero.HF.setCookie = function (name, value, date, path, domain, secure) {

	// lagacy API
	if ( typeof(arguments[0]) == 'string' ) {
		arguments[0] = {
			name: arguments[0],
			value: arguments[1],
			date: arguments[2],
			path: arguments[3], 
			domain: arguments[4],
			secure: arguments[5]
		}
	}

	// New API
	var args = Object.extend({
		name: null,
		value: null,
		date: null,
		path: '/', 
		domain: null,
		secure: false,
		escape: true
	}, arguments[0]);
	
	// expire value
	var expires = 0;

	// expires is a Date object
	if (args.date.constructor.toString().match(/function\s+Date\(/)) {
		expires = args.date;
	}
	// expires is a anonymous Object
	else if (args.date.constructor.toString().match(/function\s+Object\(/)) {
		args.date = Object.extend({ day: 0, hour: 0, min: 0, sec: 0 }, args.date);
		if ( args.date.day == 0 && args.date.hour == 0 && args.date.min == 0 && args.date.sec == 0 ) {
			expires = 0;
		}
		else {
			expires = new Date();
			Libero.HF.fixDate(expires);
			expires.setTime(expires.getTime() + (((args.date.day * 24 + args.date.hour) * 60 + args.date.min) * 60 + args.date.sec) * 1000);
		}
	}

	// set cookie
	var curCookie = args.name + '=' 
		+ ( args.escape ? escape(args.value) : args.value ) 
		+ ( expires ? '; expires=' + expires.toGMTString() : '' ) 
		+ ( args.path ? '; path=' + args.path : '' ) 
		+ ( args.domain ? '; domain=' + args.domain : '' ) 
		+ ( args.secure ? '; secure' : '' );
	document.cookie = curCookie;
}

Libero.HF.getCookie = function (name) {
	var prefix = name + "=";
	var cookieStartIndex = document.cookie.indexOf(prefix);
	if (cookieStartIndex == -1) {
		return null;
	}
	var cookieEndIndex = document.cookie.indexOf(";", cookieStartIndex + prefix.length);
	if (cookieEndIndex == -1) {
		cookieEndIndex = document.cookie.length;
	}
	return unescape(document.cookie.substring(cookieStartIndex + prefix.length,cookieEndIndex));
}

Libero.HF.deleteCookie = function (name, path, domain) {
	if (Libero.HF.getCookie(name)) {
		Libero.HF.setCookie(name, '', new Date(0), path, domain);
	}
}

Libero.HF.fixDate = function (date) {
	var base = new Date(0)
	var skew = base.getTime()
	if (skew > 0) {
		date.setTime(date.getTime() - skew);
	}
}

Libero.HF.extractSummary = function () {
	var args = Object.extend({
		elem: 'lh_email',
		max_length: 13,
		min_width: 900
	}, arguments[0]);

	if ( ! document.getElementById(args.elem) )
		return; 

	if ( document.getElementById(args.elem).title.length > args.max_length ) { 
		if ( Libero.HF.browserWidth() < args.min_width ) {
			document.getElementById(args.elem).innerHTML = document.getElementById(args.elem).title.substring(0,args.max_length - 1) + '...';
		} 
		else {
			document.getElementById(args.elem).innerHTML = document.getElementById(args.elem).title 
		}
	}
}

Libero.HF.adjustInputLabel = function () {
	var args = Object.extend({
		elem_src: 'top-query',
		elem_dest_1: 'lh_comment_col-1',
		elem_dest_2: 'lh_comment_col-2',
		offset: 20
		//,min_width: 280
	}, arguments[0]);

	if (document.getElementById) {
		var el = document.getElementById(args.elem_src);
		if ( el.clientWidth > args.min_width || args.min_width == undefined ) {
			document.getElementById(args.elem_dest_1).style.width = el.clientWidth + args.offset + 'px';
		}
	}
	Libero.HF.show(args.elem_dest_2);
}

Libero.HF.addLoadEvent = function (func) {
	var oldonload = window.onload;
	if ( typeof window.onload != 'function' ) {
    	window.onload = func;
	}
	else {
		var newonload =  function() {
			try { 
				oldonload();
			}
			catch (e) {};
			try {
				func();
			}
			catch (e) {};
		} 
		window.onload = newonload;
	}
}

Libero.HF.currentURL = function (host_only) {
	return window.location.protocol + '//' + location.host + ( host_only ? '' : (location.pathname + location.search) ); 
}

function write_HF_login (service_id) {

	if ( Libero.Stash.HF.version < '8.3.00' )
		return write_HF_login_8_2_X(service_id);

	var def_conf = {
		aiuto: {
			ret_url_raise: 1,
			url_member: 'http://aiuto.libero.it',
			cksrv_b: 'aiuto'
		},
		oroscopo: {
			cksrv_b: 'oroscopo',
			anagrafica: 1
		},
		meteo: {
			anagrafica: 1,
			cksrv_b: 'meteo'
		},
		search: {
			cksrv_b: 'search'
		}
	};

	var HF_login_el = document.getElementById('l_header_sso_login');
	var service_conf = {};
	if ( Libero.Stash.write_HF_login != undefined && typeof(Libero.Stash.write_HF_login) == 'object' ) {
		service_conf = Libero.Stash.write_HF_login;
	}
	else if ( def_conf[service_id] != undefined ) {
		service_conf = def_conf[service_id];
	}

	if (HF_login_ar[0]) {
		HF_login_el.innerHTML = '<li><a id="l_username" class="user" title="' + HF_login_ar[1] + '" href="http://selfcare.libero.it/anagrafica/?url_member=' + escape(service_conf.url_member ? service_conf.url_member : Libero.HF.currentURL(1) ) + '" onclick="return ckSrv(this,Libero.Stash.HF.cksrv.a,Libero.Stash.HF.cksrv.b,\'h_email\',null)">' + HF_login_ar[1] + '</a></li>' + 
			'<li><a class="exit" onclick="return ckSrv(this,Libero.Stash.HF.cksrv.a,Libero.Stash.HF.cksrv.b,\'h_esci\',null);" href="http://login.libero.it/logout.php?service_id=' + service_id + '&ret_url=' + escape( service_conf.ret_url ? service_conf.ret_url : Libero.HF.currentURL(service_conf.ret_url_raise) ) + '">ESCI</a></li>';
	}
	else {
		HF_login_el.innerHTML = '<li><a onclick="return ckSrv(this,Libero.Stash.HF.cksrv.a,Libero.Stash.HF.cksrv.b,\'h_registrati\',null);" href="http://registrazione.libero.it/">REGISTRATI</a></li>' + 
			'<li><a onclick="return ckSrv(this,Libero.Stash.HF.cksrv.a,Libero.Stash.HF.cksrv.b,\'h_entra\',null);" href="https://login.libero.it?service_id=' + service_id + '&ret_url=' + escape(Libero.HF.currentURL()) + '">ENTRA</a></li>';
	}
	Libero.Stash.extractSummary.min_width = 20 + document.getElementById('l_header_topbar_left').clientWidth + document.getElementById('l_header_topbar_right').clientWidth;
	Libero.HF.adjust800( Libero.objectExtend({ refresh: 1 }, Libero.Stash.extractSummary) );
}

function write_HF_login_8_2_X (service_id) {
	var def_conf = {
		aiuto: {
			ret_url_raise: 1,
			url_member: 'http://aiuto.libero.it',
			cksrv_b: 'aiuto'
		},
		oroscopo: {
			cksrv_b: 'oroscopo',
			anagrafica: 1
		},
		meteo: {
			anagrafica: 1,
			cksrv_b: 'meteo'
		},
		search: {
			cksrv_b: 'search'
		}
	};

	var HF_login_el_l = document.getElementById('HF_login_l');
	var HF_login_el_r = document.getElementById('HF_login_r');

	var service_conf = {};
	if ( Libero.Stash.write_HF_login != undefined && typeof(Libero.Stash.write_HF_login) == 'object' ) {
		service_conf = Libero.Stash.write_HF_login;
	}
	else if ( def_conf[service_id] != undefined ) {
		service_conf = def_conf[service_id];
	}

	if (HF_login_ar[0]) {
		HF_login_el_r.innerHTML = '<a href="https://login.libero.it/logout.php?service_id=' + service_id + 
			'&ret_url=' + escape( service_conf.ret_url ? service_conf.ret_url : Libero.HF.currentURL(service_conf.ret_url_raise) ) + '" class="lh_br" onclick="return ckSrv(this,Libero.Stash.HF.cksrv.a,Libero.Stash.HF.cksrv.b,\'esci\',null)">Esci</a>' +
			'<a href="http://aiuto.libero.it/" onclick="return ckSrv(this,Libero.Stash.HF.cksrv.a,Libero.Stash.HF.cksrv.b,\'h_aiuto\',null)">Aiuto</a>'	;
		HF_login_el_l.innerHTML	= ( (service_conf.url_member || service_conf.anagrafica) ? ( '<a href="http://selfcare.libero.it/anagrafica/?url_member=' + escape(service_conf.url_member ? service_conf.url_member : Libero.HF.currentURL(1) ) + '" onclick="return ckSrv(this,Libero.Stash.HF.cksrv.a,Libero.Stash.HF.cksrv.b,\'anagrafica\',null)">' + HF_login_ar[1] + '</a>' )
			: ('<div class="lh_onebox">' + HF_login_ar[1] + '</div>') )
	}
	else {
		HF_login_el_r.innerHTML = '<a href="https://login.libero.it?service_id=' + service_id + '&ret_url=' + escape(Libero.HF.currentURL()) + '" onclick="return ckSrv(this,Libero.Stash.HF.cksrv.a,Libero.Stash.HF.cksrv.b,\'accedi\',null)" class="lh_br">Accedi</a>' +
			'<a href="http://aiuto.libero.it/" onclick="return ckSrv(this,Libero.Stash.HF.cksrv.a,Libero.Stash.HF.cksrv.b,\'h_aiuto\',null)">Aiuto</a>' +
			'<a href="http://registrazione.libero.it/" onclick="return ckSrv(this,Libero.Stash.HF.cksrv.a,Libero.Stash.HF.cksrv.b,\'registrati\',null)">Registrati</a>';
	}
}


Libero.HF.autoSuggestion = function() {
	var args = Object.extend({
		as_on_id: 'libero_auto_suggest_on',
		as_off_id: 'libero_auto_suggest_off',
		click: false,
		cookie_name: 'auto_suggest',
		close: false,
		popup_id: 'l_header_topbar_popup_opzioni',
		opzioni_id: 'l_header_topbar_opzioni_lk',
		opzioni_selected_class: 'plus_selected',
		cksrv: undefined
	}, arguments[0]);
	
	var rv; 
	
	if ( Libero.HF.getCookie(args.cookie_name) == 'false' ) {
		if ( args.click ) {
			Libero.HF.setCookie(args.cookie_name, true, { day: 365 * 20	}, '/', '.libero.it');
			if ( args.cksrv != undefined && args.cksrv.constructor.toString().match(/function\s+Object\(/) ) {
				ckSrv(document.location.href, args.cksrv.a, args.cksrv.b, args.cksrv.c,'_ajax');
			}
		}			
		rv = 0;
	}
	else {
		if ( args.click ) {
			Libero.HF.setCookie(args.cookie_name, false, { day: 365 * 20 }, '/', '.libero.it');
			if ( args.cksrv != undefined && args.cksrv.constructor.toString().match(/function\s+Object\(/) ) {
				ckSrv(document.location.href, args.cksrv.a, args.cksrv.b, args.cksrv.c,'_ajax');
			}
		}
		rv = 1;
	}
	
	// Arrange div status
	if ( Libero.HF.getCookie(args.cookie_name) == 'false' ) {
		Libero.HF.show(args.as_on_id);
		Libero.HF.hide(args.as_off_id);
	}
	else {
		Libero.HF.show(args.as_off_id);
		Libero.HF.hide(args.as_on_id);
	}
	 
	if ( args.close ) {
		Libero.HF.hide(args.popup_id);
		Libero.HF.removeClass(args.opzioni_id, args.opzioni_selected_class);
	}
	
	return rv;
}

Libero.HF.simpleTemplate = function () {
	var args = Object.extend({
		tmpl: undefined,
		data: {},
		re: /\$\{([^\}]*)\}/g,
		debug: false
	}, arguments[0]);
	return args.tmpl.replace(args.re, function (m,b) { if ( args.debug) { alert(b + ': ' + args.data[b]) }; return args.data[b] || '' }); 
}

Libero.HF.addClass = function (id, classname) {
	try {
		if ( typeof(id) == 'string' ) 
			id = document.getElementById(id);
		if ( ! RegExp('\\b' + classname + '\\b').test(id.className) ) 
			id.className += ' ' + classname;
	} 
	catch (e) {}
}

Libero.HF.removeClass = function (id, classname) {
	try {
		if ( typeof(id) == 'string' ) 
			id = document.getElementById(id);
		id.className = id.className.replace(new RegExp(classname,'g'),'');
	} 
	catch (e) {}
}

Libero.HF.toggleClass = function (id, classname) {
	if ( document.getElementById(id).className.match(new RegExp(classname)) )
		Libero.HF.removeClass(id,classname);
	else 
		Libero.HF.addClass(id,classname);
}

Libero.HF.toggleTopbar = function (mode) {

	// iframe version doesn't have Libero.Stash.HF 
	try {
		// legacy header
		if ( Libero.Stash.HF.version < '8.3.00' ) {
			if ( typeof(mode) == 'undefined' ) 
			mode = /fixed/.test(document.getElementById('lh_header_top').className) ? 0 : 1; 
	
			// make fixed bar
			if ( mode == 1 ) {
				Libero.HF.removeClass(document.body,'static'); 
				Libero.HF.addClass('lh_header_top','fixed');
			}
			// make dynamic bar
			else if ( mode == 0 ) {
				Libero.HF.addClass(document.body,'static'); 
				Libero.HF.removeClass('lh_header_top','fixed');
			}
			return;
		}
	}
	catch (e) {}

	// check for inline/iframe version
	var header_el = document.getElementById('l_iframe_header_container') || document.getElementById('l_header_topbar');

	// does nothing
	if ( header_el === null ) {
		return;
	}
	
	// set default mode
	if ( typeof(mode) === 'undefined' ) 
		mode = /fixed/.test(header_el.className) ? 0 : 1; 

	// make fixed bar
	if ( mode == 1 ) {
		Libero.HF.removeClass('l_header_fixed_topbar_placeholder','static'); 
		Libero.HF.addClass(header_el,'fixed');
	}
	// make dynamic bar
	else if ( mode == 0 ) {
		Libero.HF.addClass('l_header_fixed_topbar_placeholder','static'); 
		Libero.HF.removeClass(header_el,'fixed');
	}
}

Libero.HF.browserWidth = function () {
	var w = 0;
	if( typeof(window.innerWidth) == 'number' ) 
		w = window.innerWidth;
	else if ( document.documentElement && document.documentElement.clientWidth ) 
		w = document.documentElement.clientWidth;
	else if ( document.body && document.body.clientWidth ) 
		w = document.body.clientWidth;
	return w;
}

Libero.HF.adjust800 = function () {
	var args = Object.extend({
		width: Libero.HF.browserWidth(),
		max_lenght: 15,
		min_width: 20 + document.getElementById('l_header_topbar_left').clientWidth + document.getElementById('l_header_topbar_right').clientWidth,
		elem: 'l_username',
		refresh: false
	}, arguments[0]);

	if ( Libero.Stash.adjust800 && ! args.refresh ) 
		return;

	
	Libero.HF.extractSummary( { max_length: args.max_length, min_width: args.min_width, elem: args.elem });
	Libero.HF.extractSummary( { max_length: args.max_length, min_width: args.min_width, elem: 'l_nickname' });
	
	if ( args.width < args.min_width ) {
		Libero.HF.hide('l_header_magazine');
	}
	else {
		Libero.HF.show('l_header_magazine');
	}
	if ( args.width < 770 ) {
		Libero.HF.addClass('l_header_topbar','minwidth');
	} 
	else {
		Libero.HF.removeClass('l_header_topbar','minwidth');
	}

	Libero.Stash.adjust800 = 1;
	setTimeout('Libero.Stash.adjust800 = 0',100);
};

Libero.HF.CGI = function (url) {
	var query_string;
	if ( typeof(url) === 'undefined' || typeof(url) != 'string' ) 
		url = document.location.href;
	query_string = url.split(/\?/)[1];
	if ( query_string ) {
		this.search = query_string;
		var pairs = this.search.split(/\&+/);
		for ( var i = 0; i < pairs.length; i++ ) {
			var pair = pairs[i].split('=');
			for ( var y = 0; y < pair.length; y++ ) {
				pair[y] = pair[y].replace(/\+/g, ' ');
				pair[y] = pair[y].replace(/%([a-fA-F0-9][a-fA-F0-9])/g,
					function ($0,$1) {
						$1 = $1.toUpperCase();
						var h = '0123456789ABCDEF';
						var dec = 0;
						var a = 0;
						for ( n = $1.length; n > 0; n-- ) {
							dec += h.indexOf($1.charAt(n-1)) * Math.pow(16,a);
							a++;
						}
						return String.fromCharCode(dec);
					} );
			}
			this[pair[0]] = pair[1];
		}
	}
	else {
		return {}
	}
}

Libero.HF.loadJS = function () {
	var args = Libero.objectExtend({
		document: document,
		id: 'js',
		ajax: false,
		async: false
	}, arguments[0]);

	var script = args.document.createElement('script');
	script.setAttribute('type','text/javascript');
	script.setAttribute('id', args.id );

	// get js synchronously
	if ( typeof(args.src) != 'undefined' ) {
		if ( args.ajax ) {   
			args.js = Libero.HF.makeGetPostRequest ({ url: args.src, async: args.async });
		}
		else {
			script.src = args.src;
			if ( typeof(args.onLoadHandler) == 'function' ) {
				// IE 	
				if ( script.readyState ) {
					script.onreadystatechange = function() {
						if ( script.readyState == 'loaded' || script.readyState == 'complete' ) {
							script.onreadystatechange = null;
							args.onLoadHandler();
						}
					};
				}
				else {
					script.onload = function() {
						args.onLoadHandler();
					};
				}
			 }
		}
	}

	// Hacks for MooTools
	if ( typeof(MooTools) != 'undefined' ) {
		args.js = args.js.replace(/value\s*=\s*value\.toJSON\s*\(\s*key\s*\)\s*;/,'');
	}
	script.text = args.js;

	( args.document['getElementsByTagName']('head')[0] || args.document['getElementsByTagName']('body')[0] ).appendChild(script);
}

