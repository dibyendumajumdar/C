# A C Pre-processor by Dennis Ritchie

```
From: Dennis Ritchie <d...@bell-labs.com>
Subject: Re: __LINE__ and backslash-newline
Date: 1999/01/22
Message-ID: <36A7F9CA.C8F@bell-labs.com>#1/1
X-Deja-AN: 435491428
Content-Transfer-Encoding: 7bit
References: <36A4BA62.AFA43F52@crt.umontreal.ca> <DIQJWIKdVMp2EwIG@romana.davros.org> <36A5ED25.E45ACDCF@crt.umontreal.ca> <2EEEVhMOKxp2Ew+1@romana.davros.org> <36A7AB0F.35DB829D@technologist.com>
Content-Type: text/plain; charset=us-ascii
Organization: Bell Labs, Lucent Technologies
Mime-Version: 1.0
Reply-To: d...@bell-labs.com
Newsgroups: comp.std.c

For what it's worth, my cpp (the one packaged with lcc,
and used on Plan 9 when Ken's build-in and deliberately
enfeebled version won't do) treats Tribble's example
> 
>     1:  int i =\
>     2:  \
>     3:  __L\
>     4:  IN\
>     5:  E__\
>     6:  \
>     7:  ;
> 
as expanding to
   int i =1;

It is resolute in considering the line number of
\-pasted things as belonging to the first line with
the \.  (In trying the example, I trimmed the initial
[0-9}: and white-space, of course.)

	Dennis
```
