<!-- Do not edit this file. It is automatically generated by API Documenter. -->

[Home](./index.md) &gt; [iconv-corefoundation](./iconv-corefoundation.md) &gt; [encodeSmallest](./iconv-corefoundation.encodesmallest.md)

## encodeSmallest() function

Encodes the given text, using the smallest representation supported by Core Foundation.

<b>Signature:</b>

```typescript
export declare function encodeSmallest(text: string, options?: SelectAndEncodeOptions & {
    isEncodingOk?: never;
}): TextAndEncoding;
```

## Parameters

|  Parameter | Type | Description |
|  --- | --- | --- |
|  text | <code>string</code> | The text to encode. |
|  options | <code>SelectAndEncodeOptions &amp; {</code><br/><code>    isEncodingOk?: never;</code><br/><code>}</code> | Options for encoding. |

<b>Returns:</b>

`TextAndEncoding`

The encoded text and chosen encoding.

