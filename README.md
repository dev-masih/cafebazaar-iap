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
If you really like my work and want to support me, consider donating to me with LTC, BTC or ETH. All donations are optional and are greatly appreciated. 🙏  

| LTC  | BTC | ETH |
| ------------- | ------------- | ------------- |
| ![LTC](https://user-images.githubusercontent.com/8469800/160749469-811a4395-4b71-4e03-890d-6f260c4ff36a.jpeg) | ![BTC](https://user-images.githubusercontent.com/8469800/160749502-c5b380d3-455f-483f-9f7f-d6948949259d.jpeg) | ![ETH](https://user-images.githubusercontent.com/8469800/160749569-ad99965c-ddd5-43da-8728-4b79c37fc3f5.jpeg) |
| ltc1qm6r32vjahm8wwd688enxnutks0jffc3kqg7ps5  | bc1qcuuc5r4jw38vf2eztsxag68papuwzd25chrepx  | 0x02c22832bc115933Ac11388D5A91e0990eE84667  |
  
## License  
Cafebazaar-IAP is released under the MIT License. See the [bundled LICENSE](https://github.com/dev-masih/cafebazaar-iap/blob/master/LICENSE) file for details.  
