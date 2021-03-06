<!-- Do not edit this file. It is automatically generated by API Documenter. -->

[Home](./index.md) &gt; [iconv-corefoundation](./iconv-corefoundation.md) &gt; [decode](./iconv-corefoundation.decode.md)

## decode() function

Convenience alias for [StringEncoding.decode()](./iconv-corefoundation.stringencoding.decode.md)<!-- -->.

<b>Signature:</b>

```typescript
export declare function decode(text: BufferLike, encoding: string | StringEncoding, options?: DecodeOptions): string;
```

## Parameters

|  Parameter | Type | Description |
|  --- | --- | --- |
|  text | <code>BufferLike</code> | The encoded text. |
|  encoding | <code>string &#124; StringEncoding</code> | The encoding of the <code>text</code>. May be an IANA character set name or a [StringEncoding](./iconv-corefoundation.stringencoding.md)<!-- -->. |
|  options | <code>DecodeOptions</code> | Options for decoding. |

<b>Returns:</b>

`string`

The decoded text, as a string.

