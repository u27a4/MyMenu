#include "AEConfig.h"

#include "entry.h"

#ifdef AE_OS_WIN
	#include <windows.h>
#endif

#include "AE_Macros.h"
#include "AE_EffectCBSuites.h"
#include "AEGP_SuiteHandler.h"

// This entry point is exported through the PiPL (.r file)
extern "C" DllExport AEGP_PluginInitFuncPrototype EntryPointFunc;

#define BuildMenuScript R"__(
// JSON_.stringify - from: https://github.com/douglascrockford/JSON-js
"object"!=typeof JSON_&&(JSON_={}),function(){"use strict";var p,l,n,y,e=/[\\"\u0000-\u001f\u007f-\u009f\u00ad\u0600-\u0604\u070f\u17b4\u17b5\u200c-\u200f\u2028-\u202f\u2060-\u206f\ufeff\ufff0-\uffff]/g;function t(t){return t<10?"0"+t:t}function o(){return this.valueOf()}function a(t){return e.lastIndex=0,e.test(t)?'"'+t.replace(e,function(t){var e=n[t];return"string"==typeof e?e:"\\u"+("0000"+t.charCodeAt(0).toString(16)).slice(-4)})+'"':'"'+t+'"'}"function"!=typeof Date.prototype.toJSON_&&(Date.prototype.toJSON_=function(){return isFinite(this.valueOf())?this.getUTCFullYear()+"-"+t(this.getUTCMonth()+1)+"-"+t(this.getUTCDate())+"T"+t(this.getUTCHours())+":"+t(this.getUTCMinutes())+":"+t(this.getUTCSeconds())+"Z":null},Boolean.prototype.toJSON_=o,Number.prototype.toJSON_=o,String.prototype.toJSON_=o),"function"!=typeof JSON_.stringify&&(n={"\b":"\\b","\t":"\\t","\n":"\\n","\f":"\\f","\r":"\\r",'"':'\\"',"\\":"\\\\"},JSON_.stringify=function(t,e,n){var o;if(l=p="","number"==typeof n)for(o=0;o<n;o+=1)l+=" ";else"string"==typeof n&&(l=n);if((y=e)&&"function"!=typeof e&&("object"!=typeof e||"number"!=typeof e.length))throw new Error("JSON_.stringify");return function t(e,n){var o,r,u,f,i,s=p,c=n[e];switch(c&&"object"==typeof c&&"function"==typeof c.toJSON_&&(c=c.toJSON_(e)),"function"==typeof y&&(c=y.call(n,e,c)),typeof c){case"string":return a(c);case"number":return isFinite(c)?String(c):"null";case"boolean":case"null":return String(c);case"object":if(!c)return"null";if(p+=l,i=[],"[object Array]"===Object.prototype.toString.apply(c)){for(f=c.length,o=0;o<f;o+=1)i[o]=t(o,c)||"null";return u=0===i.length?"[]":p?"[\n"+p+i.join(",\n"+p)+"\n"+s+"]":"["+i.join(",")+"]",p=s,u}if(y&&"object"==typeof y)for(f=y.length,o=0;o<f;o+=1)"string"==typeof y[o]&&(u=t(r=y[o],c))&&i.push(a(r)+(p?": ":" : ")+u);else for(r in c)Object.prototype.hasOwnProperty.call(c,r)&&(u=t(r,c))&&i.push(a(r)+(p?" : ":" : ")+u);return u=0===i.length?"{}":p?"{\n"+p+i.join(", \n"+p)+"\n"+s+"}":"{"+i.join(", ")+"}",p=s,u}}("",{"":t})})}();

__PREF_FILEPATH__ = { section: 'MyMenu', key: 'FilePath' };

function BuildMenu(source)
{
  return JSON_.stringify((function inner(items)
  {
    var menuItems = [];
    for (var i = 0; i < items.length; i++)
    {
      var item = items[i];
      var id = app.findMenuCommandId(item.command || item.name || null);
      menuItems.push({
        name: item.name || '',
        enabled: item.enabled === undefined || item.enabled,
        show: item.show === undefined || item.show,
        items: inner(item.items || []),
        func: (item.func || 'function () { app.executeCommand(' + id + '); }').toString(),
      });
    }
    return menuItems;
  })(source));
}

(function () {
  if (app.settings.haveSetting(__PREF_FILEPATH__.section, __PREF_FILEPATH__.key))
  {
    var file = new File(app.settings.getSetting(__PREF_FILEPATH__.section, __PREF_FILEPATH__.key));
    if (file.exists && file.open('r'))
    {
      var text = '';
      while (!file.eof)
      {
        text += file.readln();
      }
      file.close();

      var source = eval(text);
      if (source instanceof Array)
      {
        return BuildMenu(eval(text));
      }
    }
  }

  return BuildMenu([
    {
      name: 'Select Script File...',
      func: function ()
      {
        var file = File.openDialog('Select Menu Script...', 'JavaScript:*.jsx;*.js;*.json,All files:*.*');
        if (file !== null)
        {
            app.settings.saveSetting(__PREF_FILEPATH__.section, __PREF_FILEPATH__.key, file.fsName);
        }
      }
    }
  ]);
})();
)__"

/*
void
ShowLastErrorMessage()
{
    HLOCAL lpBuffer;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL, GetLastError(), LANG_USER_DEFAULT, (LPTSTR)&lpBuffer, 0, NULL);
    MessageBox(s_hWnd, (LPTSTR)lpBuffer, "An Error Occurred", MB_OK | MB_ICONHAND);

    LocalFree(lpBuffer);
}
*/