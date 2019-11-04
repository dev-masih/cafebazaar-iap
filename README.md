# Cafebazaar In-app purchase extension for Defold Game engine

<img src="https://raw.githubusercontent.com/dev-masih/my-media-bin/master/cafebazaar-iap/hero.jpg" alt="cafebazaar-iap banner" style="max-width:100%;" />

[مشاهده این راهنما به زبان فارسی](https://github.com/dev-masih/cafebazaar-iap/blob/master/README-fa.md)

This is a Defold [native extension](https://www.defold.com/manuals/extensions/) which provides access to In-app purchase functionality on [Cafebazaar](https://cafebazaar.ir/) platform.  
This extension is a stripped-down and modified version of [the official defold IAP extension](https://github.com/defold/extension-iap) that made compatible with Cafebazaar IAP mechanism.  

## Installation  
You can use Cafebazzar In-app purchase in your project by adding this project as a [Defold library dependency](http://www.defold.com/manuals/libraries/). Open your game.project file and in the dependencies field under project add:  

	https://github.com/dev-masih/cafebazaar-iap/archive/master.zip
  
Once added, you can use Cafebazzar In-app purchase functions using module `iapc`.  

[Official Defold game asset page for Cafebazaar In-app purchase extension](https://defold.com/assets/cafebazaar/)

## Documentations  
To learn how to use IAP please visit the [documentation page](https://defold.github.io/extension-iap/) for official defold IAP extension.  
[The manual](https://defold.com/manuals/iap/) for Defold official IAP extension is available on the official Defold site.

The only things different when you use this extension is you should use `iapc` module name instead of `iap` and `get_provider_id` function is removed due to it doesn't have any use anymore.  
Automatic finish call mechanism after each buy will obey the value of original IAP setting `(iap.auto_finish_transactions)` that is available in `game.project` file.  

If you have any issues, questions or suggestions please [create an issue](https://github.com/dev-masih/cafebazaar-iap/issues).  
  
## Contributors  
Big thanks to Defold development team. Without their efforts on the official defold IAP extension this extension wouldn't exist.  
  
## Donations  
If you really like my work and want to support me, consider donating to me with BTC or ETH. All donations are optional and are greatly appreciated. 🙏  

BTC: `1EdDfXRuqnb5a8RmtT7ZnjGBcYeNzXLM3e`  
ETH: `0x99d3D5816e79bCfB2aE30d1e02f889C40800F141`  
  
## License  
Cafebazaar-IAP is released under the MIT License. See the [bundled LICENSE](https://github.com/dev-masih/cafebazaar-iap/blob/master/LICENSE) file for details.  
