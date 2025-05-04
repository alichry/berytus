/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as fs from "node:fs/promises";
import { X509Certificate } from "@peculiar/x509";
import asn1 from "asn1.js";

const BerytusExtOID = "1.2.3.4.22.11.23";

const ASN1Entry = asn1.define('Entry', function() {
    this.seq().obj(
        this.key('spki').utf8str(),
        this.key('sksig').utf8str(),
        this.key('url').utf8str(),
    );
});

const ASN1Allowlist = asn1.define('Allowlist', function() {
    this.key("entries").seqof(ASN1Entry);
});

interface Entry {
    spki: string;
    sksig: string;
    url: string;
}

type Allowlist = Entry[];

export const parseExt = async (
    certPath = 'server/subj.cert.pem'
): Promise<Allowlist> => {
    const pem = await fs.readFile(certPath, 'utf8');
    const cert = new X509Certificate(pem);
    const ext = cert.extensions.find(e => e.type === BerytusExtOID);

    if (!ext) {
        return null;
    }
    const decoded = ASN1Allowlist.decode(Buffer.from(ext.value), 'der');
    return decoded;
}
