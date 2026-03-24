<div dir="rtl" align="right">

# افزونه پرداخت درون برنامه‌ای کافه‌بازار برای موتور بازی‌سازی دفولد

<img src="https://raw.githubusercontent.com/dev-masih/my-media-bin/master/cafebazaar-iap/hero.jpg" alt="cafebazaar-iap banner" style="max-width:100%;" />

این کتابخانه یک [افزونه محلی](https://www.defold.com/manuals/extensions/) برای موتور بازی سازی دفولد می‌باشد که به شما قابلیت استفاده از مکانیزم پرداخت درون برنامه‌ای [کافه‌بازار](https://cafebazaar.ir/) را می‌دهد.  
این افزونه نسخه سبک شده و تغییر یافته [افزونه رسمی پرداخت درون‌برنامه‌ای دفولد](https://github.com/defold/extension-iap) می‌باشد که با مکانیزم پرداخت درون‌برنامه‌ای کافه‌بازار سازگار شده است.  

## نصب  
شما با اضافه کردن افزونه پرداخت درون برنامه‌ای کافه‌بازار به شکل یک [کتابخانه وابسته دفولد](http://www.defold.com/manuals/libraries/) می‌توانید از آن استفاده کنید. برای این منظور فایل game.project را باز کنید و در قسمت dependencies لینک زیر را وارد نمایید:  

	https://github.com/dev-masih/cafebazaar-iap/archive/master.zip
  
حالا می‌توانید با استفاده از اسم ماژول `iapc` از پرداخت درون برنامه‌ای کافه‌بازار استفاده نمایید.  

[صفحه رسمی افزونه پرداخت درون برنامه‌ای کافه‌بازار در سایت دفولد](https://defold.com/assets/cafebazaar/)

## مستندات  
برای یادگیری نحوه استفاده از سیستم پرداخت درون‌برنامه‌ای لطفا به [صفحه مستندات](https://defold.github.io/extension-iap/) مربوط به افزونه رسمی پرداخت درون‌برنامه‌ای دفولد مراجعه کنید.  
[راهنمای](https://defold.com/manuals/iap/) مربوط به افزونه رسمی پرداخت درون‌برنامه‌ای دفولد در سایت رسمی دفولد موجود می‌باشد.  

تنها تفاوتی که در هنگام استفاده از این افزونه وجود دارد این می‌باشد که باید بجای استفاده از ماژول به نام `iap` از ماژون `iapc` استفاده کنید همچنین تابع `get_provider_id` به علت نداشتن هیچ کاربردی در این افزونه حذف شده است.  

مکانیزم مصرف کردن خرید اتوماتیک بعد از هر پرداخت درون‌برنامه‌ای از مقدار اصلی تنظیمات `(iap.auto_finish_transactions)` در فایل `game.project` تبعیت خواهد کرد.  

در صورت برخورد به مشکل یا طرح سوال و پیشنهاد در مورد این افزونه داشتید لطفا [با ساخت issue](https://github.com/dev-masih/cafebazaar-iap/issues) اطلاع دهید.  
  
## حمایت  
در صورتی که از این افزونه رضایت داشتید و می‌خواهید از من حمایت کنید می‌توانید حمایت‌های مالی خودتون رو از طریق درگاه [زرین‌پال](https://zarinp.al/@dev-masih) واریز نمایید. حمایت مالی به هیچ عنوان اجباری نیست. از حامیان محترم بسیار سپاسگزارم.
همچنین در صورت دلخواه می‌توانید با استفاده از کریپتو نیز از من حمایت کنید.

<p align="left">
  <img src="https://raw.githubusercontent.com/dev-masih/my-media-bin/refs/heads/master/donation-links/donation-link-bitcoin.svg" width="180" style="margin-right: 20px;" alt="Bitcoin Donation">
  <img src="https://raw.githubusercontent.com/dev-masih/my-media-bin/refs/heads/master/donation-links/donation-link-ethereum.svg" width="180" alt="Ethereum Donation">
</p>

## لایسنس  
این افزونه تحت لایسنس MIT منتشر شده. برای مشاهده جزئیات بیشتر به [فایل لایسنس](https://github.com/dev-masih/cafebazaar-iap/blob/master/LICENSE) مراجعه کنید.  

</div>
