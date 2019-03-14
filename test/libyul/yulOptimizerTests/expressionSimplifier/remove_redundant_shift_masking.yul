// EVMVersion: >constantinople
{
    let a := and(0xff, shr(248, calldataload(0)))
    let b := and(shr(248, calldataload(0)), 0xff)
}
// ----
// expressionSimplifier
// {
//     let a := shr(248, calldataload(0))
//     let b := shr(248, calldataload(0))
// }
