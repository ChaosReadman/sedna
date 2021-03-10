(:PARAMS:)

declare function se:uuid() as xs:string external;
let $a := se:uuid()

return
<result>
{$a}
</result>
