# MiLedger

WebSocket server documentation
---

Basics

1. Host to connect (default): 127.0.0.1:8081 for http and websocket
2. User can change default port and resolve IP-address, for example to avoid NAT limitations
3. There are no TLS support yet, so use `http:` and `ws:` protocols accordingly
4. Good practice is to show on client raw transaction hash that the user can compare on the device screen before accept
   signing

Message format
---

Message format for exchange is json:

- type: non-null string type of message
- value: nullable string with message or value (individual for each operation)
- payload: object with key-value format (individual for each operation)

Message types:
"types" are divided on 3 types:

- `event_`: events only works in websocket, they're just inform client without any special request
- `action_`: client request to do something
- `result_`: result of request

```cpp
enum type_t {
    // Simple device state. Client receive it when device state has been changed or just new client connected
    event_device_state_changed = (uint8_t) 0x00,
    // if client send message to sign transaction, ledger ask for user action (accepted or rejected)
    event_user_action_required,
    // result of user action
    event_user_action_result,

    // common error
    event_error,

    // request for getting ledger address
    action_get_address,
    // result with requested address
    result_get_address,

    // request for signing raw transaction
    action_sign_tx,
    // result with signed transaction
    result_sign_tx,

    // get current device state
    action_get_device_state,
    // result with device state
    result_get_device_state,
};
```

Usage
---
As websocket is asynchronous by its nature, all requests to websocket are asynchronous too.  
But there is one thing: Ledger can handle only one request at time. By this reason, server (http and ws)  
works in blocking mode, this means you can send (for example) two `action_get_address` requests at time, but results
will back sequentially.  
Exception is: `action_get_device_state`. This method getting result from internal state and do not interact with Ledger
directly.

HTTP server works in synchronous mode.  
This means if you are sending `action_sign_tx` request, you'll get response only after all processes will complete.
Therefore, you should be prepared to disable http request timeout, as response can block request for undefined time.


Examples
---
Websocket handles only json request, format shown in [Message format](#message-format).

Websocket get address
---------------------
Request:

```json
{
  "type": "action_get_address"
}
```

Result:

```json
{
  "type": "result_get_address",
  "value": "Mx0011001100110011001100110011001100110011"
}
```

Websocket sign tx
---

```json
{
  "type": "action_sign_tx",
  "value": "<raw_tx_hex_string>"
}
```

What is the **raw tx** (pseudocode):

```
tx = create_tx();
// set data to tx
tx.set_gas_price(1);
tx.set_send_to(Mx...);
// etc

// rlp encoded transaction without signature
rlp_encoded_tx = encode_rlp_without_signature(tx);

// 32 bytes sha3k hash of rlp encoded transaction
// use this hash to sign via ledger
raw_tx_hash = sha3k(rlp_encoded_tx)

```

While you're signing transaction, server will send extra event messages.  
For tx signing, it is 2 special events:

- `event_user_action_required`: simple event that represents server intention to ask for user interaction: accept or
  reject transaction
- `event_user_action_result`: result of user action

Event `event_user_action_required`:

```json
{
  "type": "event_user_action_required"
}
```

Event `event_user_action_result`:

```json
{
  "type": "event_user_action_result",
  "value": "<status_string>",
  "payload": {
    "status_code": <status_code_integer>
  }
}
```

Available status strings:

- success
- no_status_result
- invalid_parameter
- user_rejected
- unknown_error

Status codes (in hex):

- 0x9000: success
- 0x9001: no_status_result
- 0x6985: user_rejected
- 0x6b01: invalid_parameter
- other codes: unknown_error

Only 2 of these statuses related directly to user action: `user_rejected` and `success`. If other code occurred, please,
report me.

Result of tx signing:

```json
{
  "type": "result_sign_tx",
  "value": null,
  "payload": {
    "r": "f60c9929077b67a80799d5bf82db8dbd64eb56ae67f3b03ee108a7d3327e80fc",
    "s": "00a6a81e975b3b97356ddcbd28040071323cba4f8ab87f4bf6932b5b8831e4a2",
    "v": "1c"
  }
}
```

If user rejected signing transaction, message `result_sign_tx` will not come. Instead, you should check `user_rejected`
status from event `event_user_action_result`.

Also, you can send wrong data, for example: invalid raw transaction. Server has special error message with
type: `event_error`.  
Format of this message is unified for all available errors:

```json
{
  "type": "event_error",
  "value": "error_message",
  "payload": {
    // payload can contains extra fields
  }
}
```

Websocket get device state
--------------------------
Request:

```json
{
  "type": "action_get_device_state",
  "value": "<device_state_string>"
}
```

Available device states:

- `DISCONNECTED`: ledger is not connected to computer or cannot detect usb device
- `APP_NOT_OPENED`: ledger is connected, everything's fine, but Minter app still not opened.
- `PERMISSION_ERROR`: most likely linux-specific error because **udev** rules are not configured. Simple solution: run
  MiLedger as `sudo`, right
  way: [LedgerHQ support page](https://support.ledger.com/hc/en-us/articles/360019868977-Fix-USB-connection-issues-with-Ledger-Live)
- `APP_OPENED`: everything's fine, you can send all requests

If you try to send device-specific request and device is in invalid state, server will return error:

```json
{
  "payload": {
    "type_code": 3,
    "state": "<device_state_string>"
  },
  "type": "event_error",
  "value": "Can't proceed request: device is in invalid state: <device_state_string>"
}
```

HTTP Server request
-------------------

HTTP server just needed for cases, when you need to block interaction, or if you have no possibility to handle
asynchronous responses.  
It support only 3 endpoints:

- `/action_get_device_state`
- `/action_get_address`
- `/action_sign_tx?tx=RAW_TX_HEX_VALUE`

Response of http server are in the same json format as websocket messages.

