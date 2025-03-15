# electricity.jinja
This macro can, with the help of sensors reporting known e.g. energy prices, calculate when, within a given timeframe, the
cheapest /most expensive period will occur. 

## Basic use
The makro can be called adding just the sensor_today parameter (providing the sensor has a raw_today and even a raw_tomorrow attribute). As such the macro is out-of-the-box capable of handling sensor from e.g. the Nordpool and Energi Data Service integrations. However, as can be seen below, the macro can ingest from other sensors as well, provided necessary information is included in the pararmeters. More about that later.<br/>
Note that any missing parameter will have a default value. Parameters can be added in any order providing the key (the name of the parameter) is included. If parameter name is omitted,
the correct order of parameters is mandatory to ensure correct parsing. E.g.:<br/>
  {{- PeriodPrice("sensor.edssensor", duration=timedelta(minutes=90)) -}} 

## Parameters
Required = *
| Parameter | Description |
|-----------|-------------|
| * sensor_today       |(dict / string) The sensor that contains pricing data for today.|
| sensor_tomorrow      |(dict / string) The sensor that contains pricing data for tomorrow.<br/>Note that if sensor_today is a string, this sensor will default to sensor_today, otherwise it will default to dict()|
| sensor_forecast      |(dict / string) The sensor that contains pricing data for forecast.<br/>Note that if sensor_today is a string, this sensor will default to sensor_today, otherwise it will default to dict()|
| earliestDatetime       |(DateTime) Exact date/time of start of window for which pricing will be analyzed.|
| latestDatetime         |(DateTime) Exact date/time of end of window for which pricing will be anaylyzed.|
| duration               |(Integer / TimeDelta) Duration of the expected usage period in minutes (if integer).|
| returnFirst            |(Boolean) Default to true to use first occurrence of the lowest / highest price, otherwise use the last.|
| timeAdherence          |(String) Influences the macros behaviour when handling earliest / latest time of window.|
| mode                   |(String) Defines what is returned by the macro.|
| hint                   |(String) Will be returned as part of the result.|
| usageWindow            |(Integer) Number (1-60) of minutes that each part of the expectedConsumption list is valid for. If omitted, 60 will be used.|
| expectedConsumption    |(Integer / list) A list [] of numbers, or a single (total) number with the expected consumption.|
| validateData           |(String) Allows validation of price data.|


### sensor_xxxxxx
Possible value formats (* indicates optional):
- "entity_id" (string)
- dict(data:[], *timeTag="string", *priceTag="string)

A value of "" or dict() is used to indicate 'empty value' and means that this particular sensor and subsequent sensor(s) data will be ignored. The sensors order is sensor_today -> sensor_tomorrow -> sensor_forecast.
If "entity_id" format is used, it is assumed that the sensor has an attribute that (dependent on the corresponding key _sensor_xxxxxx_) will be either _raw_today_, _raw_tomorrow_ or _forecast_. If only _sensor_today_ is listed, the other two sensors will use _sensor_today_, and this requirement is fulfilled. Also note that if the attributes are named differently, a dict _must_ be provided instead as this is the only way to have control over the necessary attribute.<br/>
In the dict, the _data_ must be formatted as a list of sets, i.e. [{key_date=date1, key_price=price1}, {key_date=date2, key_price=price2}, ...]. If timeTag and price are omitted, macro attempts to see of the default values (_timeTag = start_ and _priceTag = price_) work. If that fails it will attemtp to identify suitable candites for the key names using the first set in the list:<br/>
- A value will be identified as a date, if it is a datetime (number, string or datetime object) and is either not a number or a number (if date was converted to an epoch; https://www.epochconverter.com/) that is above a sanity date (default is today at midnight). Since prices is the only other value expected in each set, this is considered a rather safe assumption. The lowest value found fulfilling the (number + sanity date) rule will have the corresponding key used as timeTag.
- A value will be identified as a number, if it either _is_ a number or if int filter return something usable. _( X |int(1) == X int(2)_ ) is a check that will ensure this, since an invalid value will return default and thus never result in the final result being equal :)

### earliestDatetime / latestDatetime
A datatime object may be provided for start / end of the window during which cheapest / most expensive price is calculated. _earliestDatetime_ will default to _now()_ if omitted. _latestDatetime_ will be set to _earliestDatetime_ + _defaultPeriodHrsNum_ (24) hours if omitted. Furthermore, see _timeAdherence_ for how that will affect _earliestDatetime_ / _latestDatetime_.

### duration
Possible value formats:
- number
- timedelta()

If a timedelta formatted value is provided, duration is rounded to whole minutes.

### timeAdherence
**default**&nbsp;&nbsp;&nbsp;adjusts time to 'now' if time is in the past (default value)<br/>
**strict**&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;do not adjust time (start / end) and return empty result if window is in the past<br/>
**forced**&nbsp;&nbsp;&nbsp;&nbsp;do not adjust time (start / end) and return result even if window is in the past

### mode
Defines what is returned by the macro (default being, well the default). Note that all data returned will be formatted as a string.<br/>
default / details / detailscompact return a json (see below under **Returns**. _details_ will contain same data as default + explain information + priceWindow + usageWindow. _detailscompact_ will simply compact the explain information so that adjacent time periods with same pricing are combined into one (this a more compact read)./<br/>
cheapPrice / cheapStart / expensiveStart / isCheapNow / expensivePrice / expensiveStart / expensiveStart / isExpensiveNow will return a single string containing the requested value (suited for use in a Helper Template Sensor).

### Hint
String to be returned as part of the result. This could (e.g.) be the name of the integration providing data. If _sensor_xxxxxx_ is an entity_id, hint will return the friendly name if omitted.

### expectedConsumption / usageWindow
Possible value formats:
- number
- []

If only one number is provided, this is assumed to be be total consumption and will be distributed evenly across duration.<br/>
If a list [] is provided, the consumption for that particular period of time (defined by the _usageWindow_), or timeslice, will be the number listed. See further below under **Slicing / expectedConsumption**

### validateData
validateData will permit checking of the data provided by _sensor_xxxxxx_. A valid check means that all dates in today / tomorrow / forecast is spaced at the same intervals, and that the intervals between today -> tomorrow and tomorrow -> forecast are also spaced at this interval<br/>
**ignore**&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;do not validate data (default)<br/>
**prune**&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;check for error, stop at first and keep data until this point<br/>
**stopOnError**&nbsp;&nbsp;&nbsp;check for error, stop at first and stop further processing
 
## Returned data
Macro returns a STRING(!) based on the MODE setting. In case of a mode of default / details / detailsCompact, the returned string will
be json formatted and must be converted using the _from_json_ filter.

When mode is default / details / detailsCompact, the returned value (passed through the from_json filter) may contain following fields:

"*" = only included if a corresponding period was found
| Parameter | Description |
|-----------|-------------|
| cheap / expensive          | JSON containing information relating to cheapest / most expensive period. Each contains:<br/>* **startTime**: Datetime string of when time starts<br/>* **endTime**: Datetime string of when time ends<br/>**price**: Price if 1 (or the under expectedConsumption mentioned) unit (e.g. kWh) is used for the duration. None if no period found<br/>**isCheapNow**/**isExpensiveNow**: none if no cheap/expensive time period found, otherwise true / false dependent on whether right now is the cheapest/most expensive period |
| window                     | JSON containing information relating to used window:<br/>**earliestDatetime**: Datetime string of the earliest time used when looking for cheap / expensive<br/>**latestDatetime**: Datetime string of the latest time used when looking for cheap / expensive<br/>**duration**: The duration of the time window looked for (in minutes) |
| mode                       | Name of the mode used |
| status                     | Result of the operation. If not 'ok', then this is an warning / error |
| hint                       | none or string as provided when  macro was called |

If mode is details / detailsCompact, follwing data will also be available under the relevant JSONs
| Parameter | Description |
|-----------|-------------|
| explain                    | <ins>Under cheap/expensive JSON</ins><br/>Explaination for calculation to reach price.<br/>Set of [{ start, minutes, unitPrice, quantity, sum },...] covering the whole duration. Start defines when each subset starts, length in minutes of the subset, unitPrice is the corresponding price per hour and quantity is what is consumed in the corresponding slice. Sum = unitPric * quantity and is the estimated cost for the slice.|
| prices                     | The full list of prices used for calculation |
| validate                   | <ins>Under window JSON</ins><br/>JSON with following possible fields:<br/>**validationMode** = Lower case version of _validateData_<br/>**check**          = Status of last validation<br/>**kept**           = Number of kept records<br/>**lastGood**       = Last good record<br/>**invalid**        = First bad record|

If _detailsCompact_ is used, the list in _explain_ will be compacted in accordance to prics irrespectively of the _usageWindow_ or _expectedConsumption_ settings.
<br/>
<br/>
For any other mode, the returned value will be a string with the following content:

| Mode      | Description |
|-----------|-------------|
|cheapPrice     | String value of price if 1 unit (e.g. 1 kWh) is used each hour during cheapest time. If expectedConsumption is supplied, then that is used instead of 1. None if no period found |
|cheapStart     | Datetime string of when cheapest time starts |
|cheapEnd       | Datetime string of when cheapest time ends |
|expensivePrice | String value of price if 1 kW is used each hour during most expensive time. If expectedConsumption are supplied, then that is used instead of 1. None if no period found |
|expensiveStart | Datetime string of when most expensive time starts |
|expensiveEnd   | Datetime string of when most expensive time ends |

## Advanced use
### Multiple sensors
The macro has the ability to get pricing from multiple sensors, although with the requirement of them covering adjacent periods. This allows for instance collection of day-ahead pricing for today / tomorrow / forecast. I.e. sensor1 can provide todays pricing, and sensor2 can provide tomorrows; e.g. this could be the case when using [Strømligning](https://github.com/MTrab/stromligning). If forecast data is available (e.g. you have a sensor that pulls data from [Carnots API](https://www.carnot.dk/), then this data can also be used. However, there are some requirements that must be met:
- Data obtained from these (up to) three sensors may not overlap, but must be adjacent.
- The prices must be valid for the same duration (e.g. hour).
- The name (entity_id) of the sensor must be listed in sensor_today / sensor_tomorrow / sensor_forecast. Note:
  - if sensor_forecast is set to empty, sensor_forecast data is ignored
  - if sensor_tomorrow is set to empty, both data for sensor_tomorrow as well as sensor_forecast are ignored
  - Default value of sensor_tomorrow / sensor_forecast is the value of sensor_today
- In the entity, an attribute is expected named raw_today / raw_tomorrow / forecast. The attributes must exist in the corresponding sensor_today / sensor_tomorrow / sensor_forecast. If the attributes are named differently, a dict must be supplied explicitly naming the entity and the attribute.
- Each of the attributes must contain a list of sets. Inside a list, pairs must be named alike. The default names are 'start' and 'price' and can be overridden using the parameters timeTag and priceTag (if the dict is used). If nothing is provided to the macro macro will try to guess the names.

### Search mode
timeAdherence controls how the macro uses earliestDatetime / latestDatetime. 
- If timeAdherence is ommitted or set to 'default', the macro will adjust earliestDatetime / latestDatetime to match a earliestDatetime equal to <ins>now</ins>.
- If set to strict, the earliestDatetime / latestDatetime will not be adjusted. If matching time window is found, the information will only be returned if this is in the present. Otherwise an empty set is returned.
- If set to forced, the earliestDatetime / latestDatetime will not be adjusted. Information will be returned irrespectively of this being in the past or present.

### Data returned
returnFirst will define (if true) to return the earliest possible match. If false, the latest match will be returned.

### Prices
By default the macro will detect for how long the prices are valid. This is done by taking the time difference of two first price set found. Should only one price be found, 1 hour is assumed. This allows for a later adjustment in the pricing mode (e.g. pricing per half hour instead of hourly) without having to change the macro.

### Slicing / expectedConsumption
If data is available on how much power (normally) is expected consumed, expectedConsumption can be added when calling the macro. To allow some variation, an hour is divided into smaller parts (slices). Each part of the expectedConsumption list is
attributed to a slice, and is only used once during a calculation. If a number is provided instead of a list, the number is assumed to be the total consumption and will be distributed evenly across the duration.<br/>
The usageWindow must be a whole number between 1 and 60 minutes. This duration is applied to each of the slices listed in expectedConsumption. If the total usageWindow for all slices is less than the duration time, an error will be returned. If expectedConsumption is a number instead of a list, the macro will create its own expectedConsumption list and spread the provided consumption evenly across the duration (considering the usageWindow requested). If expectedConsumption is empty, **an usage of 1 unit (e.g. kWh) is assumed per price window**. If no usageWindow is provided, a window of 60 minutes is assumed; the usageWindow is not coupled to the priceWindow allowing maximum freedom to define each of these.
| usageWincow | expectedConsumption | Description |
|-------------|-----------|-------------|
| included (1-60) | number | _duration_ is interrnally split into small slices with the same length as _usage_window_ and _expectedConsumption_ is split equally amongst these slices|
| included (1-60) | list   | If _usageWindow_ multiplied by length of _expectedConsumption_ does not exceed duration, an error is raised|
| included (1-60) | not included | Usage is assumed to be 1 unit (e.g. kWh) per _priceWindow_. No checks are necessary. |
| not included | list   | _usageWindow_ is assumed to be 60mins. If _usageWindow_ multiplied by by length of _expectedConsumption_ does not exceed duration, an error is raised. |
| not included | number | _expectedConsumption_ is assumed evenly distributed across _duration_. An internal split of _expectedConsumption_ is created based on the default length of _usageWindow_|
| not included | not included |  Usage is assumed to be 1 unit (e.g. kWh) per _priceWindow_. No checks are necessary. |

**Note:**<br/>
- For each hour there are up to three points upon calculation is based; on the hour, the current minute and modulus (duration % hour) before the hour. The majority of iterations will have 2 or (on rare occassions) 1 data points per hour. Other factor that affect number of calculations are, lower _usageWindow_ lower _priceWindow_ and longer _duration. . If in doubt, test the configuration in _Template_ under _Developer tools_ by adding a _**{%- from 'Electricity.jinja' import PeriodPrice -%}**_ in front of the code (see examples below) to get an impression of how the macro performs.
- If duration is one hour or less, slicing will provide little if any beenfit. If duration is low (typically 2 hours or less), there _can_ be a gain by applying a _usageWindow_ and a _expectedConsumption_, however, often adjacent price windows will have a difference in pricing so low that the potential saving can be measured in a few cents. Exception from this is for price windows that go low -> high prive ande vice versa. Dependent on where is the usage cycle power is consumed, viable windows may be returned that otherwise could have been rejected. E.g. a window with 50 cent / kWh windows is followed by a 150 cent / kWh window. If duration is 2 hours and consumed power is 4 kWH, this would normally be considered a bad choice since the estimated price would be 400 cents for the duration. However, if measurenments showed that 80% of power was consumed within the first 60 mins, the estimated price would change to 280.<br/>For longer duration, and especially where the consumed power varies to a considerable degree over time, benefits may be seen from using _usageWindow_ and a _expectedConsumption_. However, do also consider the validity of the data; e.g. forecast may not provide as valuable result with a low _usageWindow_ vs day-ahead and a low _usageWindow_.


## Code examples
### Get the cheapest hour, within the next 48 hours (ie. using defaults)
            {% from 'Electricity.jinja' import PeriodPrice %}
            {%- set edsSensor         = "sensor.energidataservice" -%}
            {{- PeriodPrice(edsSensor, 
                            hint="Energi Data Service") | from_json -}}
 
### Dishwasher must be run at cheapest hour, but be completed by 06:00 tomorrow
            {% from 'Electricity.jinja' import PeriodPrice %}
            {%- set edsSensor         = "sensor.energidataservice" -%}
            {%- set earliestDatetime  = now() -%}
            {#- Add 20min for the normal cycle of dishwasher cooling off and opening door -#}
            {%- set duration=  20 + states('sensor.dishwasher_remaining_time') | int(90) -%}
            {#- Dishwasher is precoded to start 04:30 at the latest, thus providing 'failsafe' -#}
            {%- set latestDatetime    = earliestDatetime + timedelta(minutes=states('sensor.dishwasher_start_time') | int(8*60) + duration) -%}
            {#- Get data from EnergiDataService, ignore forecasted values -#}
            {{- PeriodPrice(edsSensor, sensor_forecast="", 
                            earliestDatetime=earliestDatetime, latestDatetime=latestDatetime, duration=duration, 
                            hint="Energi Data Service") | from_json -}}
              
### Dishwasher must be run at cheapest hour, but be completed by 06:00 tomorrow. Cascade two integrations for data
            {% from 'Electricity.jinja' import PeriodPrice %}
            {%- set edsSensor         = "sensor.energidataservice" -%}
            {%- set earliestDatetime  = now() -%}
            {#- Add 20min for the normal cycle of dishwasher cooling off and opening door -#}
            {%- set duration= 20 + states('sensor.dishwasher_remaining_time') | int(90) -%}
            {#- Dishwasher is precoded to start 04:30 at the latest, thus providing 'failsafe' -#}
            {%- set latestDatetime    = earliestDatetime + timedelta(minutes=states('sensor.dishwasher_start_time') | int(8*60) + duration)  -%}
            {%- set latestDatetimeStr = latestDatetime | string -%}

            {#- Get data from EnergiDataService, ignore forecasted values -#}
            {%- set resultEDS = PeriodPrice(edsSensor, sensor_forecast="", earliestDatetime=earliestDatetime, latestDatetime=latestDatetime, duration=duration, hint="Energi Data Service") | from_json -%}
            {%- if resultEDS.cheapPrice != none and resultEDS.latestDatetime >= latestDatetime |string -%}
              {{ resultEDS }}
            {%- else -%}
              {%- set slSensor = "sensor.stromligning_current_price_vat" %}
              {%- set slSensor_tomorrow= "binary_sensor.stromligning_tomorrow_available_vat" %}
              {%- set resultSL = PeriodPrice(sensor_today=dict(data=state_attr(slSensor, "prices"), timeTag="start", priceTag="price"),
                                             sensor_tomorrow=dict(data=state_attr(slSensor_tomorrow, "prices"), timeTag="start", priceTag="price"),
                                             sensor_forecast="",
                                             earliestDatetime=earliestDatetime, latestDatetime=latestDatetime, duration=duration,
                                             hint="Strømligning") | from_json %}
              {%- if resultSL.cheapPrice != none and resultSL.latestDatetime >= latestDatetime |string -%}
                {{ resultSL }}
              {%- else -%}
                {#- Last resort. If neither integration returns usable data, just use what EnergiDataService offered -#}
                {{ resultEDS }}
              {%- endif -%}
            {%- endif -%}

The last example can be extended by using additional integrations (e.g. Nordpool).


### Get cheapest hour within the next 12 hours
            {%- from 'Electricity.jinja' import PeriodPrice -%}
            {%- set edsSensor           = "sensor.energidataservice" -%}
            {%- set earliestDatetime    = now() -%}
            {%- set latestDatetime      = earliestDatetime + 12*60 -%}
            {%- set duration  = 60 -%}
            {#- Get data from EnergiDataService, ignore forecasted values -#}
            {{- PeriodPrice(edsSensor, sensor_forecast="", 
                            earliestDatetime=earliestDatetime, latestDatetime=latestDatetime, duration=duration, 
                            hint="Energi Data Service") | from_json -}}

### Get cheapest period within the next 12 hours, where power consumption expected is 1.2kWh, distributed (in 15 minute intervals) to be 0.4kWh, 0.3kWh, 0.18kWh, 0.17kWh, 0.1 kWh and 0.05kWh.
            {%- from 'Electricity.jinja' import PeriodPrice -%}
            {%- set edsSensor           = "sensor.energidataservice" -%}
            {%- set earliestDatetime    = now() -%}
            {%- set latestDatetime      = earliestDatetime + timedelta(hours=12) -%}
            {%- set duration  = timedelta(minutes=90 ) -%}
            {#- Get data from EnergiDataService, ignore forecasted values -#}
            {{- PeriodPrice(edsSensor, sensor_forecast="", 
                            earliestDatetime=earliestDatetime, latestDatetime=latestDatetime, duration=duration, 
                            usageWindow=15, expectedConsumption=[0.4,0.3,0.18,0.17,0.1,0.05], 
                            hint="Energi Data Service") | from_json -}}
<br/>
<br/>

## Frequently asked questions
1. **_Can I use the macro in a Template Helper?_**<br/>
   Yes. A good choice is the Template Senor and then use one of the code examples above as a guideline. Remember to add a mode (e.g. _mode="cheapStart"_) to the call of PeriodPrice macro. Currently states are limited to 256 characters so forgetting this part will result in an error. Further remember to omit filtering (_| from_json_) from the returned result. While Home Assistant may ignore such a filter, the data returned is <ins>not</ins> json formatted and thus an error may arise at a later stage should the Home Assistant developers decide to enforce a stricter syntax.
1. **_I keep getting noWindowFound_**
  Check following
    - If _sensor_xxxxxx_ contains the string that is the actual name of the sensor, does the sensor have a _raw_today_ / _raw_tomorrow_ / _forecast attribute_? If so, does each of these consist of pair with _hour_ and _price_? If the answer is no to either of these, you will have to use a dict in the following format: _dict(data=state_attr(sensor, attribute), timeTag="tag_str", priceTag="price_str")_
    - If sensor_xxxxxx is a dict, please check for proper formatting and naming. Cross check with the actual sensor (using Developer Tools -> States in Home Assistant).
    - Has _latestDatetime_ been passed when using _mode=strict_? If so, either change _latestDatetime_ or change _mode_ to a different setting
    - Are the sensors missing day-ahead for tomorrow, and is time so close to midnight that duration is longer than time until midnight? If so, either decrease duration or see attempt to rectify the missing day-ahead information


# day-ahead_fallback.snip
Template for a sensor to be added to configuration.yaml. Sensor will allow fallback between the Home Assistant integrations Energi Data Service -> Strømligning -> Nordpool in case the day-ahead prices are not updated. Please note that there is no validation of the suitability of the data from each integration (that is the responisibility of the integration) nor is there any handling of missing information (i.e. forecast info when using Strømigning / Nordpool or tarifs when using Nordpool).<br/>
The template can be extended with other integrations if so needed.
