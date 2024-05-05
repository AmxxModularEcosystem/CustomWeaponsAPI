#if defined __UTILS_ArrayTrieUtils_included
    #endinput
#endif
#define __UTILS_ArrayTrieUtils_included

#include <amxmodx>

stock ArraySizeSafe(const Array:iArr) {
    if (iArr == Invalid_Array) {
        return 0;
    }

    return ArraySize(iArr);
}

stock TrieSizeSafe(const Trie:iTrie) {
    if (iTrie == Invalid_Trie) {
        return 0;
    }

    return TrieGetSize(iTrie);
}

stock ArrayDestroySafe(Array:a) {
    if (a != Invalid_Array) {
        ArrayDestroy(a);
    }
}

stock TrieDestroySafe(Trie:t) {
    if (t != Invalid_Trie) {
        TrieDestroy(t);
    }
}

#define ArrayForeachArray(%1:%4=>%2[%3]) \
    if(%1 != Invalid_Array) \
        for(new %2[%3], %4 = 0; %4 < ArraySizeSafe(%1); %4++) \
            if(ArrayGetArray(%1, %4, %2))

#define ArrayForeachCell(%1:%2=>%3) \
    if(%1 != Invalid_Array) \
        for(new any:%3, %2 = 0; %2 < ArraySizeSafe(%1); %2++) \
            if((%3 = ArrayGetCell(%1, %2)) == %3)

#define ArrayForeachString(%1:%4=>%2[%3]) \
    if(%1 != Invalid_Array) \
        for(new %2[%3], %4 = 0; %4 < ArraySizeSafe(%1); %4++) \
            if(ArrayGetString(%1, %4, %2, charsmax(%2)))

stock Array:ArrayCreateIfNotCreated(&Array:iArr, const iCellSize = 1, const iReserved = 1) {
    if (iArr == Invalid_Array) {
        return iArr = ArrayCreate(iCellSize, iReserved);
    } else {
        return iArr;
    }
}

stock Array:TrieCreateIfNotCreated(&Trie:iTrie) {
    if (iTrie == Invalid_Trie) {
        return iTrie = TrieCreate();
    } else {
        return iTrie;
    }
}

stock Array:ArrayDestroyIfEmpty(&Array:iArr) {
    if (!ArraySizeSafe(iArr)) {
        ArrayDestroy(iArr);
    }
    
    return iArr;
}

stock Array:ArrayMergeCells(const Array:a1, const Array:a2) {
    new Array:aMerged = ArrayClone(a1);
    ArrayResize(aMerged, ArraySizeSafe(a1) + ArraySizeSafe(a2));

    if (a2 == Invalid_Array) {
        return aMerged;
    }
    
    for (mew i = 0; iSize = ArraySizeSafe(a2); i < iSize; i++) {
        ArrayPushCell(aMerged, ArrayGetCell(a2, i));
    }

    return aMerged;
}
