The [electricity.dashboard](https://github.com/henriklund/ha_public/tree/main/dashboard/plotly/electricity.dashboard) is an example of how to visualize electricity princing in a HA dashboard using Plotly:
![image](https://github.com/user-attachments/assets/e8a899f2-fce9-48c9-b343-ffc7e98249d2)
</br>
</br>
Following sensor are expected / used:

|Sensor|Description|
|------|-----------|
|elpris_billigste_periode|Expected to contain attributes endTime & duration|
|elpris_dyreste_periode  |Expected to contain attributes endTime & duration|
|aktuel_elpris_tariffer_aendres|If tariffs are reduced upon reaching a specific amount of kWh consumed, this Helper must return the date (of when tariffs are reduced or reset|
|sensor.aktuel_elpris|Helper to calculate current price based consumed electricity vs tariffs. Expects a float|
|sensor.combined_day_ahead|Sensor that contains the pricing from (e.g.) Stromligning ordered with raw_today, raw_tomorrow and forecast attributes of which each is calculated in accordance to the current state of tariff reduction (i.e. if reduction is in effect or not)|
|sensor.stromligning_current_price_vat|Sensor from [Strømligning integration](https://github.com/MTrab/stromligning)|
|sensor.stromligning_tariffer|Sensor from [Strømligning integration](https://github.com/MTrab/stromligning)|

