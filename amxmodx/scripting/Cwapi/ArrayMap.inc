#if defined __UTILS_ArrayMap_included
    #endinput
#endif
#define __UTILS_ArrayMap_included

#include amxmodx

enum ArrayMap{
    Array:AM_Arr,
    Trie:AM_Map,
}
#define ArrayMap(%1) \
    %1[ArrayMap]

#define DefineArrayMap(%1) \
    new ArrayMap(%1)
    
#define DefineStaticArrayMap(%1) \
    static ArrayMap(%1)

stock InitArrayMap(AM[ArrayMap], const CellSize = 1, const Reserved = 32) {
    AM[AM_Arr] = ArrayCreate(CellSize, Reserved);
    AM[AM_Map] = TrieCreate();
}

stock ArrayMapPushArray(AM[ArrayMap], const any:Input[], const Key[], Size = -1) {
    new Id = ArrayPushArray(AM[AM_Arr], Input, Size);
    TrieSetCell(AM[AM_Map], Key, Id, true);
    return Id;
}

stock ArrayMapPushString(AM[ArrayMap], const Input[], const Key[]) {
    new Id = ArrayPushString(AM[AM_Arr], Input);
    TrieSetCell(AM[AM_Map], Key, Id, true);
    return Id;
}

stock ArrayMapPushCell(AM[ArrayMap], any:Input, const Key[]) {
    new Id = ArrayPushCell(AM[AM_Arr], Input);
    TrieSetCell(AM[AM_Map], Key, Id, true);
    return Id;
}

stock ArrayMapGetIndex(AM[ArrayMap], const Key[]) {
    new Id = -1;
    TrieGetCell(AM[AM_Map], Key, Id);
    return Id;
}


#define ArrayMapForeach(%1,%2) \
    if(ArrayMapCreated(%1)) \
        for(new %2 = 0; %2 < ArrayMapSize(%1); %2++)

#define ArrayMapForeachArray(%1:%4=>%2[%3]) \
    if(ArrayMapCreated(%1)) \
        for(new %2[%3], %4 = 0; %4 < ArrayMapSize(%1); %4++) \
            if(ArrayMapGetiArray(%1, %4, %2))


#define ArrayMapCreated(%1) \
    (%1[AM_Arr] != Invalid_Array)

#define ArrayMapDestroy(%1) \
    if(ArrayMapCreated(%1)){ \
        ArrayDestroy(%1[AM_Arr]); TrieDestroy(%1[AM_Map]); \
    }

#define ArrayMapSize(%1) \
    (ArrayMapCreated(%1) ? ArraySize(%1[AM_Arr]) : 0)

#define ArrayMapHasKey(%1,%2) \
    (%2[0] && TrieKeyExists(%1[AM_Map], %2))
    

#define ArrayMapGetiArray(%1,%2,%3) \
    ArrayGetArray(%1[AM_Arr], _:(%2), %3)

#define ArrayMapGetArray(%1,%2,%3) \
    ArrayMapGetiArray(%1, ArrayMapGetIndex(%1, %2), %3)

#define ArrayMapSetArray(%1,%2,%3) \
    ArraySetArray(%1[AM_Arr], ArrayMapGetIndex(%1, %2), %3)
    

#define ArrayMapGetCell(%1,%2) \
    ArrayGetCell(%1[AM_Arr], ArrayMapGetIndex(%1, %2))

#define ArrayMapSetCell(%1,%2) \
    ArraySetCell(%1[AM_Arr], ArrayMapGetIndex(%1, %2), %3)


#define ArrayMapGetString(%1,%2,%3) \
    ArrayGetString(%1[AM_Arr], ArrayMapGetIndex(%1, %2), %3)

#define ArrayMapSetString(%1,%2,%3) \
    ArraySetString(%1[AM_Arr], ArrayMapGetIndex(%1, %2), %3)
