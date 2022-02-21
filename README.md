# ttgo-tdisplay-crypto-ticker

Simple crypto ticker, that grabs prices and 24h changes of two desired cryptos from Binance API in desired real currency, grabs real currency exchange rate of two desired currencies from Open ER API, calculates your balance in original currency and desired. Then it displays it on TFT display of TTGo T-Display, alongside with time and date grabbed from NTP server.

In my code it grabs ETH and BTC prices in EUR, grabs exchange rate EUR-PLN and calculates my balance in EUR and PLN.

Crypto prices and time refreshes every 5 seconds, exchange rates refresh every 60 minutes.

## Configuration
1. lines 16 and 17 - your WiFi credentials
2. line 23 - your balance in desired cryptos (position corresponds to position in cryptos table)
3. line 39 - two desired cryptos
4. line 41 - two desired currencies (crypto price will be grabbed in currency on first position, then calculated to currency on second position).
