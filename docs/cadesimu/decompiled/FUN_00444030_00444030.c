// FUN_00444030 @ 00444030

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00444030(undefined4 param_1,int param_2,int param_3)

{
  bool bVar1;
  char cVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  int extraout_ECX;
  int iVar6;
  undefined4 *puVar7;
  bool bVar8;
  float10 fVar9;
  undefined4 auStackY_1063c [16];
  int *piStackY_105fc;
  int iStackY_105f8;
  int iStackY_105f4;
  int iStackY_105f0;
  int iStackY_105ec;
  int *piStackY_105e8;
  int iStackY_105e4;
  int iStackY_105e0;
  int iStackY_105dc;
  int iStackY_105d8;
  undefined4 uStackY_105d4;
  int iStackY_105d0;
  int iStackY_105cc;
  int iStackY_105c8;
  int *piStackY_105c4;
  int iStackY_105c0;
  int *piStackY_105bc;
  int iStackY_105b8;
  undefined4 uStackY_105b4;
  char acStackY_105b0 [8];
  char acStackY_105a8 [52];
  char acStackY_10574 [8];
  char acStackY_1056c [52];
  int iStackY_10538;
  char acStackY_10534 [52];
  char acStackY_10500 [2];
  char cStackY_104fe;
  undefined4 uStackY_104bc;
  int iStackY_104b8;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_008d07fc;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_008ce5b0();
  puVar7 = auStackY_1063c;
  for (iVar6 = 0x418b; iVar6 != 0; iVar6 = iVar6 + -1) {
    *puVar7 = 0xcccccccc;
    puVar7 = puVar7 + 1;
  }
  iVar6 = thunk_FUN_0042c200();
  Ordinal_1042();
  iVar3 = Ordinal_3010();
  iVar4 = Ordinal_3010();
  param_2 = param_2 + iVar3;
  param_3 = param_3 + iVar4;
  if (DAT_00bd966c == 1) {
    if ((*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 8) &&
       (*(int *)(extraout_ECX + 0x158a4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0)) {
      DAT_00bd94a8 = 1;
      Ordinal_734();
      DAT_00bd9608 = *(int *)(extraout_ECX + 0x23364 + *(int *)(extraout_ECX + 0xb4) * 4);
      DAT_00bd9600 = *(undefined4 *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4);
      DAT_00bd9604 = *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4);
      DAT_00bd95d0 = *(undefined4 *)(extraout_ECX + 0x1d5a4 + *(int *)(extraout_ECX + 0xb4) * 4);
      DAT_00bd95d4 = *(undefined4 *)(extraout_ECX + 0x1f4e4 + *(int *)(extraout_ECX + 0xb4) * 4);
      Ordinal_734();
      Ordinal_734();
      Ordinal_734();
      Ordinal_734();
      Ordinal_734();
      Ordinal_734();
      Ordinal_734();
      Ordinal_734();
      Ordinal_734();
      thunk_FUN_008cb2e0();
      local_8 = 0;
      iVar3 = Ordinal_1862();
      if (iVar3 == 1) {
        Ordinal_734();
        *(int *)(extraout_ECX + 0x23364 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9608;
        *(undefined4 *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9600;
        *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9604;
        *(undefined4 *)(extraout_ECX + 0x1d5a4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95d0;
        *(undefined4 *)(extraout_ECX + 0x1f4e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95d4;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_454();
        ftol();
        ftol();
        ftol();
        ftol();
        Ordinal_4811();
        Ordinal_877();
        Ordinal_3312();
        Ordinal_2640();
        if (DAT_00bd9608 == 0) {
          uVar5 = ftol();
          *(undefined4 *)(extraout_ECX + 0x9d24 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
          uVar5 = ftol();
          *(undefined4 *)(extraout_ECX + 0xbc64 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
        }
        else {
          iStackY_105cc = -DAT_00bd9608 / 0xc;
          uVar5 = ftol();
          *(undefined4 *)(extraout_ECX + 0x9d24 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
          iStackY_105d0 = -DAT_00bd9608 / 0xc;
          uVar5 = ftol();
          *(undefined4 *)(extraout_ECX + 0xbc64 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
        }
      }
      local_8 = 0xffffffff;
      thunk_FUN_00439510();
    }
    if (((0 < *(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4)) &&
        (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) < 8)) &&
       (*(int *)(extraout_ECX + 0x158a4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0)) {
      DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
      DAT_00bd9604 = *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4);
      DAT_00bd95cc = *(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4);
      thunk_FUN_0088e3a0();
      local_8 = 1;
      iVar3 = Ordinal_1862();
      if (iVar3 == 1) {
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9604;
        Ordinal_3312();
      }
      local_8 = 0xffffffff;
      thunk_FUN_0044dd20();
    }
    if ((*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 9) &&
       (*(int *)(extraout_ECX + 0x158a4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0)) {
      Ordinal_734();
      thunk_FUN_00878ca0();
      local_8 = 2;
      iVar3 = Ordinal_1862();
      if (iVar3 == 1) {
        iStackY_105b8 = Ordinal_719();
        local_8._0_1_ = 3;
        if (iStackY_105b8 == 0) {
          uStackY_105d4 = 0;
        }
        else {
          uStackY_105d4 = thunk_FUN_008745e0();
        }
        uStackY_105b4 = uStackY_105d4;
        local_8 = CONCAT31(local_8._1_3_,2);
        *(undefined4 *)(iVar6 + 0x13c + DAT_00bd9668 * 4) = uStackY_105d4;
        cVar2 = thunk_FUN_00874ca0();
        if (cVar2 == '\0') {
          if (DAT_00bd9580 == 0) {
            Ordinal_1136();
          }
          if (DAT_00bd9580 == 1) {
            Ordinal_1136();
          }
          if (DAT_00bd9580 == 2) {
            Ordinal_1136();
          }
          if (DAT_00bd9580 == 3) {
            Ordinal_1136();
          }
          if (DAT_00bd9580 == 4) {
            Ordinal_1136();
          }
          Ordinal_1510();
          local_8 = 0xffffffff;
          thunk_FUN_00433ce0();
          goto LAB_0044bd9a;
        }
        Ordinal_474();
        Ordinal_1510();
        iVar3 = *(int *)(extraout_ECX + 0xb4);
        iVar4 = ftol();
        *(int *)(extraout_ECX + 0x9d24 + *(int *)(extraout_ECX + 0xb4) * 4) =
             *(int *)(extraout_ECX + 0x5ea4 + iVar3 * 4) + iVar4;
        iVar3 = *(int *)(extraout_ECX + 0xb4);
        iVar4 = ftol();
        *(int *)(extraout_ECX + 0xbc64 + *(int *)(extraout_ECX + 0xb4) * 4) =
             *(int *)(extraout_ECX + 0x7de4 + iVar3 * 4) + iVar4;
        Ordinal_734();
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9668;
        Ordinal_3312();
      }
      local_8 = 0xffffffff;
      thunk_FUN_00433ce0();
    }
    if ((999 < *(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4)) &&
       (*(int *)(extraout_ECX + 0x158a4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0)) {
      DAT_00bd95cc = *(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4);
      Ordinal_734();
      Ordinal_734();
      Ordinal_734();
      Ordinal_734();
      Ordinal_734();
      Ordinal_734();
      Ordinal_734();
      Ordinal_734();
      Ordinal_734();
      Ordinal_734();
      Ordinal_734();
      DAT_00bd95d0 = *(undefined4 *)(extraout_ECX + 0x1d5a4 + *(int *)(extraout_ECX + 0xb4) * 4);
      DAT_00bd95d4 = *(undefined4 *)(extraout_ECX + 0x1f4e4 + *(int *)(extraout_ECX + 0xb4) * 4);
      DAT_00bd95d8 = *(undefined4 *)(extraout_ECX + 0x57f24 + *(int *)(extraout_ECX + 0xb4) * 4);
      DAT_00bd95dc = *(undefined4 *)(extraout_ECX + 0x59e64 + *(int *)(extraout_ECX + 0xb4) * 4);
      DAT_00bd95e0 = *(undefined4 *)(extraout_ECX + 0x5bda4 + *(int *)(extraout_ECX + 0xb4) * 4);
      DAT_00bd95e4 = *(undefined4 *)(extraout_ECX + 0x5dce4 + *(int *)(extraout_ECX + 0xb4) * 4);
      DAT_00bd95e8 = *(undefined4 *)(extraout_ECX + 0x5fc24 + *(int *)(extraout_ECX + 0xb4) * 4);
      DAT_00bd95ec = *(undefined4 *)(extraout_ECX + 0x61b64 + *(int *)(extraout_ECX + 0xb4) * 4);
      DAT_00bd95f0 = *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4);
      DAT_00bd95f4 = *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4);
      DAT_00bd95f8 = *(undefined4 *)(extraout_ECX + 0x67924 + *(int *)(extraout_ECX + 0xb4) * 4);
      DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
      DAT_00bd9600 = *(undefined4 *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4);
      Ordinal_734();
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x420) {
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        DAT_00bd9640 = *(undefined4 *)(extraout_ECX + 0x57f28 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd9644 = *(undefined4 *)(extraout_ECX + 0x59e68 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd9648 = *(undefined4 *)(extraout_ECX + 0x5bda8 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd964c = *(undefined4 *)(extraout_ECX + 0x5dce8 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd9650 = *(undefined4 *)(extraout_ECX + 0x5fc28 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd9654 = *(undefined4 *)(extraout_ECX + 0x61b68 + *(int *)(extraout_ECX + 0xb4) * 4);
      }
      if (((*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x425) ||
          (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2776)) ||
         (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2777)) {
        DAT_00bd95f8 = *(undefined4 *)(extraout_ECX + 0x67924 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
      }
      bVar8 = false;
      bVar1 = false;
      if (((*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 4000) ||
          (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0xfa9)) ||
         ((*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0xfaa ||
          ((*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0xfb2 ||
           (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0xfb3)))))) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd9604 = *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4);
        bVar1 = true;
        thunk_FUN_0088e3a0();
        local_8 = 4;
        iVar3 = Ordinal_1862();
        bVar8 = iVar3 == 1;
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9604;
        local_8 = 0xffffffff;
        thunk_FUN_0044dd20();
      }
      if ((*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x426) ||
         (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x427)) {
        bVar1 = true;
        Ordinal_734();
        thunk_FUN_00406c30();
        local_8 = 5;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
          Ordinal_734();
        }
        local_8 = 0xffffffff;
        thunk_FUN_00406f00();
      }
      if ((((*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x275d) ||
           (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x275e)) ||
          (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2762)) ||
         ((*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2763 ||
          (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2760)))) {
        Ordinal_3312();
        bVar1 = true;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        DAT_00bd95d8 = *(undefined4 *)(extraout_ECX + 0x57f24 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95dc = *(undefined4 *)(extraout_ECX + 0x59e64 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95e0 = *(undefined4 *)(extraout_ECX + 0x5bda4 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95e4 = *(undefined4 *)(extraout_ECX + 0x5dce4 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95e8 = *(undefined4 *)(extraout_ECX + 0x5fc24 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95ec = *(undefined4 *)(extraout_ECX + 0x61b64 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95f0 = *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95f4 = *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        thunk_FUN_0088efa0();
        local_8 = 6;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
          Ordinal_734();
          Ordinal_734();
          Ordinal_734();
          Ordinal_734();
          Ordinal_734();
          Ordinal_734();
          Ordinal_734();
          Ordinal_734();
          Ordinal_734();
          *(undefined4 *)(extraout_ECX + 0x57f24 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95d8
          ;
          *(undefined4 *)(extraout_ECX + 0x59e64 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95dc
          ;
          *(undefined4 *)(extraout_ECX + 0x5bda4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95e0
          ;
          *(undefined4 *)(extraout_ECX + 0x5dce4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95e4
          ;
          *(undefined4 *)(extraout_ECX + 0x5fc24 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95e8
          ;
          *(undefined4 *)(extraout_ECX + 0x61b64 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95ec
          ;
          *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f0
          ;
          *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f4
          ;
          *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044de10();
      }
      if (((*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x429) ||
          (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x42a)) ||
         (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x42b)) {
        Ordinal_3312();
        bVar1 = true;
        Ordinal_734();
        thunk_FUN_00897430();
        local_8 = 7;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
          Ordinal_734();
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044e170();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x1f57) {
        bVar1 = true;
        thunk_FUN_008b2200();
        local_8 = 8;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044e450();
      }
      if ((((*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2330) ||
           (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2331)) ||
          (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2758)) ||
         (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2759)) {
        bVar1 = true;
        thunk_FUN_008b2200();
        local_8 = 9;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044e450();
      }
      if ((0x232b < *(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4)) &&
         (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) < 0x2330)) {
        bVar1 = true;
        thunk_FUN_008c8690();
        local_8 = 10;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044e670();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2337) {
        bVar1 = true;
        thunk_FUN_008ca270();
        local_8 = 0xb;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044e870();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x1b69) {
        bVar1 = true;
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        thunk_FUN_008c8690();
        local_8 = 0xc;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
          *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044e670();
      }
      if ((*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x1b6c) ||
         (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x1b6d)) {
        bVar1 = true;
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        thunk_FUN_008c8690();
        local_8 = 0xd;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
          *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044e670();
      }
      if ((6999 < *(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4)) &&
         (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) < 0x1b5a)) {
        bVar1 = true;
        thunk_FUN_00898e90();
        local_8 = 0xe;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044e980();
      }
      if (((0x1d4b < *(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4)) &&
          (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) < 0x1d7e)) ||
         ((0x1d7e < *(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) &&
          (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) < 0x1db6)))) {
        Ordinal_3312();
        bVar1 = true;
        thunk_FUN_00888210();
        local_8 = 0xf;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044ebf0();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x420) {
        bVar1 = true;
        thunk_FUN_008a59e0();
        local_8 = 0x10;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044eea0();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x425) {
        bVar1 = true;
        thunk_FUN_008a79c0();
        local_8 = 0x11;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044f070();
      }
      if ((9999 < *(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4)) &&
         (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) < 0x2732)) {
        bVar1 = true;
        thunk_FUN_00898e90();
        local_8 = 0x12;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044e980();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2776) {
        bVar1 = true;
        thunk_FUN_008cbcb0();
        local_8 = 0x13;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044f1c0();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2777) {
        bVar1 = true;
        DAT_00bd95f4 = *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        thunk_FUN_00405930();
        local_8 = 0x14;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f4;
        local_8 = 0xffffffff;
        thunk_FUN_00405c10();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x274f) {
        bVar1 = true;
        thunk_FUN_00891240();
        local_8 = 0x15;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044f2b0();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x275b) {
        bVar1 = true;
        thunk_FUN_00891240();
        local_8 = 0x16;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044f2b0();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2752) {
        bVar1 = true;
        thunk_FUN_008c8690();
        local_8 = 0x17;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044e670();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2761) {
        bVar1 = true;
        thunk_FUN_008c8690();
        local_8 = 0x18;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044e670();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2753) {
        bVar1 = true;
        thunk_FUN_008c8690();
        local_8 = 0x19;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044e670();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2754) {
        bVar1 = true;
        thunk_FUN_008c8690();
        local_8 = 0x1a;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044e670();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2755) {
        bVar1 = true;
        thunk_FUN_008ca270();
        local_8 = 0x1b;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044e870();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2336) {
        bVar1 = true;
        thunk_FUN_008a8ae0();
        local_8 = 0x1c;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044f4f0();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2338) {
        bVar1 = true;
        thunk_FUN_00891240();
        local_8 = 0x1d;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_0044f2b0();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x27d8) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        bVar1 = true;
        thunk_FUN_00894800();
        local_8 = 0x1e;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        local_8 = 0xffffffff;
        thunk_FUN_0044f620();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x27d9) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        bVar1 = true;
        thunk_FUN_008a2a80();
        local_8 = 0x1f;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        local_8 = 0xffffffff;
        thunk_FUN_0044f830();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x27da) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        DAT_00bd9604 = *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95f0 = *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95f4 = *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        bVar1 = true;
        thunk_FUN_008960e0();
        local_8 = 0x20;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        if (DAT_00bd95fc == 0) {
          uVar5 = ftol();
          *(undefined4 *)(extraout_ECX + 0xc4) = uVar5;
          uVar5 = ftol();
          *(undefined4 *)(extraout_ECX + 200) = uVar5;
          uVar5 = ftol();
          *(undefined4 *)(extraout_ECX + 0xcc) = uVar5;
          uVar5 = ftol();
          *(undefined4 *)(extraout_ECX + 0xd0) = uVar5;
        }
        if ((DAT_00bd95fc == 1) || (DAT_00bd95fc == 2)) {
          uVar5 = ftol();
          *(undefined4 *)(extraout_ECX + 0xc4) = uVar5;
          uVar5 = ftol();
          *(undefined4 *)(extraout_ECX + 200) = uVar5;
          uVar5 = ftol();
          *(undefined4 *)(extraout_ECX + 0xcc) = uVar5;
          uVar5 = ftol();
          *(undefined4 *)(extraout_ECX + 0xd0) = uVar5;
        }
        if (DAT_00bd95fc == 3) {
          uVar5 = ftol();
          *(undefined4 *)(extraout_ECX + 0xc4) = uVar5;
          uVar5 = ftol();
          *(undefined4 *)(extraout_ECX + 200) = uVar5;
          uVar5 = ftol();
          *(undefined4 *)(extraout_ECX + 0xcc) = uVar5;
          uVar5 = ftol();
          *(undefined4 *)(extraout_ECX + 0xd0) = uVar5;
        }
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0xdba4 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0xfae4 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9604;
        *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f0;
        *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f4;
        local_8 = 0xffffffff;
        thunk_FUN_0044fa40();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x27e6) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        DAT_00bd9604 = *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95f0 = *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95f4 = *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        bVar1 = true;
        thunk_FUN_00891f00();
        local_8 = 0x21;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9604;
        *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f0;
        *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f4;
        local_8 = 0xffffffff;
        thunk_FUN_0044fb10();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x27db) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        DAT_00bd9604 = *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95f0 = *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95f4 = *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        bVar1 = true;
        thunk_FUN_008960e0();
        local_8 = 0x22;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0xc4) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 200) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0xcc) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0xd0) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0xdba4 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0xfae4 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9604;
        *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f0;
        *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f4;
        local_8 = 0xffffffff;
        thunk_FUN_0044fa40();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x27e7) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        DAT_00bd9604 = *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95f0 = *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95f4 = *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        bVar1 = true;
        thunk_FUN_00891f00();
        local_8 = 0x23;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9604;
        *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f0;
        *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f4;
        local_8 = 0xffffffff;
        thunk_FUN_0044fb10();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x27e1) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        DAT_00bd9604 = *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95f0 = *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95f4 = *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        bVar1 = true;
        thunk_FUN_008960e0();
        local_8 = 0x24;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9604;
        *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f0;
        *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f4;
        local_8 = 0xffffffff;
        thunk_FUN_0044fa40();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x27e2) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        DAT_00bd9600 = *(undefined4 *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd9604 = *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        Ordinal_734();
        bVar1 = true;
        thunk_FUN_00891f00();
        local_8 = 0x25;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        local_8 = 0xffffffff;
        thunk_FUN_0044fb10();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x27e3) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        DAT_00bd9604 = *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95f0 = *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95f4 = *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        bVar1 = true;
        thunk_FUN_008960e0();
        local_8 = 0x26;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9604;
        *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f0;
        *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f4;
        local_8 = 0xffffffff;
        thunk_FUN_0044fa40();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x27e4) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        DAT_00bd9600 = *(undefined4 *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd9604 = *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        Ordinal_734();
        bVar1 = true;
        thunk_FUN_00891f00();
        local_8 = 0x27;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        local_8 = 0xffffffff;
        thunk_FUN_0044fb10();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x27e5) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        DAT_00bd9600 = *(undefined4 *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd9604 = *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        Ordinal_734();
        bVar1 = true;
        thunk_FUN_00891f00();
        local_8 = 0x28;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        local_8 = 0xffffffff;
        thunk_FUN_0044fb10();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x27de) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        DAT_00bd9604 = *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95f0 = *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95f4 = *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        bVar1 = true;
        thunk_FUN_008a4360();
        local_8 = 0x29;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0xc4) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 200) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0xcc) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0xd0) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0xdba4 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0xfae4 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9604;
        *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f0;
        *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f4;
        local_8 = 0xffffffff;
        thunk_FUN_0044fc30();
      }
      if ((((*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x27dc) ||
           (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x27dd)) ||
          (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x27df)) ||
         (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x27e0)) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        bVar1 = true;
        thunk_FUN_008a4360();
        local_8 = 0x2a;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0xc4) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 200) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0xcc) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0xd0) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0xdba4 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0xfae4 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
        uVar5 = ftol();
        *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        local_8 = 0xffffffff;
        thunk_FUN_0044fc30();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x27ec) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        DAT_00bd9604 = *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95f0 = *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4);
        DAT_00bd95f4 = *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        bVar1 = true;
        thunk_FUN_0088c440();
        local_8 = 0x2b;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        *(undefined4 *)(extraout_ECX + 0x1b664 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9604;
        *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f0;
        *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f4;
        local_8 = 0xffffffff;
        thunk_FUN_0044fe40();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2904) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        bVar1 = true;
        thunk_FUN_0089b1a0();
        local_8 = 0x2c;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        local_8 = 0xffffffff;
        thunk_FUN_00450030();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2905) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        bVar1 = true;
        thunk_FUN_0089b1a0();
        local_8 = 0x2d;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        local_8 = 0xffffffff;
        thunk_FUN_00450030();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2906) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        bVar1 = true;
        thunk_FUN_0089b1a0();
        local_8 = 0x2e;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        local_8 = 0xffffffff;
        thunk_FUN_00450030();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2907) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        bVar1 = true;
        thunk_FUN_0089b1a0();
        local_8 = 0x2f;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        local_8 = 0xffffffff;
        thunk_FUN_00450030();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2908) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        bVar1 = true;
        thunk_FUN_0089b1a0();
        local_8 = 0x30;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        local_8 = 0xffffffff;
        thunk_FUN_00450030();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2909) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        bVar1 = true;
        thunk_FUN_0089b1a0();
        local_8 = 0x31;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        local_8 = 0xffffffff;
        thunk_FUN_00450030();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x290a) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        bVar1 = true;
        thunk_FUN_0089b1a0();
        local_8 = 0x32;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        local_8 = 0xffffffff;
        thunk_FUN_00450030();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x290b) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        bVar1 = true;
        thunk_FUN_0089b1a0();
        local_8 = 0x33;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        local_8 = 0xffffffff;
        thunk_FUN_00450030();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x290c) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        bVar1 = true;
        thunk_FUN_0089b1a0();
        local_8 = 0x34;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        local_8 = 0xffffffff;
        thunk_FUN_00450030();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x290d) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        bVar1 = true;
        thunk_FUN_0089b1a0();
        local_8 = 0x35;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        local_8 = 0xffffffff;
        thunk_FUN_00450030();
      }
      if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2936) {
        DAT_00bd95fc = *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4);
        Ordinal_734();
        bVar1 = true;
        thunk_FUN_0089b1a0();
        local_8 = 0x36;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        local_8 = 0xffffffff;
        thunk_FUN_00450030();
      }
      if (!bVar1) {
        thunk_FUN_00879230();
        local_8 = 0x37;
        iVar3 = Ordinal_1862();
        if (iVar3 == 1) {
          bVar8 = true;
        }
        local_8 = 0xffffffff;
        thunk_FUN_00450860();
      }
      if (bVar8) {
        *(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95cc;
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        Ordinal_734();
        *(undefined4 *)(extraout_ECX + 0x1d5a4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95d0;
        *(undefined4 *)(extraout_ECX + 0x1f4e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95d4;
        *(undefined4 *)(extraout_ECX + 0x57f24 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95d8;
        *(undefined4 *)(extraout_ECX + 0x59e64 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95dc;
        *(undefined4 *)(extraout_ECX + 0x5bda4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95e0;
        *(undefined4 *)(extraout_ECX + 0x5dce4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95e4;
        *(undefined4 *)(extraout_ECX + 0x5fc24 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95e8;
        *(undefined4 *)(extraout_ECX + 0x61b64 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95ec;
        *(undefined4 *)(extraout_ECX + 0x63aa4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f0;
        *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f4;
        *(undefined4 *)(extraout_ECX + 0x67924 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f8;
        *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95fc;
        Ordinal_734();
        if (((*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 1000) ||
            (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x3e9)) &&
           (0 < *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4))) {
          if (*(int *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4) == 0) {
            *(undefined4 *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4) = 0;
          }
          if (*(int *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4) == 1) {
            thunk_FUN_0043b4c0();
            *(undefined4 *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4) = 0;
          }
          if (*(int *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4) == 2) {
            thunk_FUN_0043b4c0();
            thunk_FUN_0043b4c0();
            *(undefined4 *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4) = 0;
          }
          if (*(int *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4) == 3) {
            thunk_FUN_0043b4c0();
            thunk_FUN_0043b4c0();
            thunk_FUN_0043b4c0();
            *(undefined4 *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4) = 0;
          }
          if (*(int *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4) == 4) {
            thunk_FUN_0043cd20();
            *(undefined4 *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4) = 0;
          }
          if (*(int *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4) == 5) {
            thunk_FUN_0043b4c0();
            thunk_FUN_0043cd20();
            *(undefined4 *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4) = 0;
          }
          if (*(int *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4) == 6) {
            thunk_FUN_0043b4c0();
            thunk_FUN_0043b4c0();
            thunk_FUN_0043cd20();
            *(undefined4 *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4) = 0;
          }
          if (*(int *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4) == 7) {
            thunk_FUN_0043b4c0();
            thunk_FUN_0043b4c0();
            thunk_FUN_0043b4c0();
            thunk_FUN_0043cd20();
            *(undefined4 *)(extraout_ECX + 0x19724 + *(int *)(extraout_ECX + 0xb4) * 4) = 0;
          }
        }
        if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0xfab) {
          *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 6;
          *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0xf;
          if (8 < *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4)) {
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x15;
          }
          if (0x11 < *(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4)) {
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1b;
          }
        }
        if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x1b6a) {
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 1)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 2)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1b;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x30;
          }
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 3) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 4)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 5)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x2d;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x4b;
          }
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 6) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 7)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 8)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x39;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x60;
          }
        }
        if (((*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x1b6b) ||
            (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x1b6c)) ||
           (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x1b6d)) {
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 1)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 2)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1b;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1e;
          }
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 3) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 4)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 5)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x2d;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x2d;
          }
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 6) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 7)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 8)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x39;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x3c;
          }
        }
        if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x177f) {
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 1)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 2)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1b;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x36;
          }
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 3) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 4)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 5)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x2d;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x54;
          }
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 6) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 7)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 8)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x39;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x69;
          }
        }
        if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x1780) {
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 1)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 2)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1b;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x27;
          }
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 3) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 4)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 5)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x2d;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x39;
          }
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 6) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 7)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 8)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x39;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x4b;
          }
        }
        if ((*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x1781) ||
           (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x1782)) {
          if ((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0) ||
             ((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 1 ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 2)))) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0xf;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1e;
          }
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 3) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 4)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 5)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1b;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x39;
          }
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 6) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 7)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 8)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x24;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x4b;
          }
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 9) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 10)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0xb)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1b;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1e;
          }
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0xc) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0xd)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0xe)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x33;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x39;
          }
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0xf) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x10)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x11)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x45;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x4b;
          }
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x12) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x13)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x14)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x15;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1e;
          }
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x15) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x16)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x17)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x27;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x39;
          }
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x18) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x19)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x1a)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x33;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x4b;
          }
        }
        if ((*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x1783) ||
           (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x1784)) {
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0) ||
              (((((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 1 ||
                  (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 2)) ||
                 (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 9)) ||
                ((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 10 ||
                 (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0xb)))) ||
               (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x12)))) ||
             ((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x13 ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x14)))) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1b;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1b;
          }
          if (((((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 3) ||
                (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 4)) ||
               (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 5)) ||
              (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0xc ||
                (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0xd)) ||
               ((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0xe ||
                ((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x15 ||
                 (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x16))))))
              )) || (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x17))
          {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x33;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x33;
          }
          if (((((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 6) ||
                (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 7)) ||
               (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 8)) ||
              (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0xf ||
                (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x10)) ||
               ((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x11 ||
                ((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x18 ||
                 (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x19))))))
              )) || (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x1a))
          {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x4b;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x4b;
          }
        }
        if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x434) {
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 0) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 1)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 2)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1b;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0xc;
          }
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 3) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 4)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 5)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x30;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x15;
          }
          if (((*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 6) ||
              (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 7)) ||
             (*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xb4) * 4) == 8)) {
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x42;
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1b;
          }
        }
        if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x420) {
          Ordinal_734();
          Ordinal_734();
          Ordinal_734();
          Ordinal_734();
          Ordinal_734();
          Ordinal_734();
          Ordinal_734();
          *(undefined4 *)(extraout_ECX + 0x57f28 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9640
          ;
          *(undefined4 *)(extraout_ECX + 0x59e68 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9644
          ;
          *(undefined4 *)(extraout_ECX + 0x5bda8 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9648
          ;
          *(undefined4 *)(extraout_ECX + 0x5dce8 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd964c
          ;
          *(undefined4 *)(extraout_ECX + 0x5fc28 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9650
          ;
          *(undefined4 *)(extraout_ECX + 0x61b68 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd9654
          ;
        }
        if ((*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x425) ||
           (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2777)) {
          *(undefined4 *)(extraout_ECX + 0x659e4 + *(int *)(extraout_ECX + 0xb4) * 4) = DAT_00bd95f4
          ;
          Ordinal_734();
        }
        if (*(int *)(extraout_ECX + 0x2024 + *(int *)(extraout_ECX + 0xb4) * 4) == 0x2776) {
          Ordinal_734();
          iStackY_104b8 = Ordinal_2640();
          if (0xc < iStackY_104b8) {
            uStackY_104bc = 0;
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4c770) {
              uStackY_104bc = 0x40000000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4d298) {
              uStackY_104bc = 0x40800000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4c828) {
              uStackY_104bc = 0x40c00000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4d288) {
              uStackY_104bc = 0x40e00000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4d278) {
              uStackY_104bc = 0x41100000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4d268) {
              uStackY_104bc = 0x41400000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4d258) {
              uStackY_104bc = 0x41600000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4d248) {
              uStackY_104bc = 0x41800000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4d238) {
              uStackY_104bc = 0x41900000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4c9c8) {
              uStackY_104bc = 0x41a00000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4d228) {
              uStackY_104bc = 0x41b00000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4d218) {
              uStackY_104bc = 0x41c00000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4d208) {
              uStackY_104bc = 0x41d00000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4d1f8) {
              uStackY_104bc = 0x41e00000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4d1e8) {
              uStackY_104bc = 0x41f00000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4d1d8) {
              uStackY_104bc = 0x42000000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4d1c8) {
              uStackY_104bc = 0x42080000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4d1b8) {
              uStackY_104bc = 0x42100000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4d1a8) {
              uStackY_104bc = 0x42180000;
            }
            if (*(double *)(iVar6 + 0x58) == _DAT_00b4c7e0) {
              uStackY_104bc = 0x42200000;
            }
            uVar5 = ftol();
            *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = uVar5;
            Ordinal_454();
            Ordinal_4811();
            Ordinal_877();
            Ordinal_3312();
          }
        }
        if ((((((DAT_00bd95cc == 0x1d4c) || (DAT_00bd95cc == 0x1d52)) || (DAT_00bd95cc == 0x1d58))
             || ((DAT_00bd95cc == 0x1d60 || (DAT_00bd95cc == 0x1d66)))) ||
            ((DAT_00bd95cc == 0x1d6c || ((DAT_00bd95cc == 0x1d7f || (DAT_00bd95cc == 0x1d81)))))) ||
           ((DAT_00bd95cc == 0x1d87 ||
            ((((DAT_00bd95cc == 0x1d92 || (DAT_00bd95cc == 0x1d98)) || (DAT_00bd95cc == 0x1d9e)) ||
             (DAT_00bd95cc == 0x1da4)))))) {
          Ordinal_3312();
          *(undefined4 *)(extraout_ECX + 0xdba4 + *(int *)(extraout_ECX + 0xb4) * 4) = 0xffffffee;
          *(undefined4 *)(extraout_ECX + 0xfae4 + *(int *)(extraout_ECX + 0xb4) * 4) = 0xfffffffd;
          *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 9;
          *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0xf;
          *(undefined4 *)(extraout_ECX + 0xd4) =
               *(undefined4 *)(extraout_ECX + 0xdba4 + *(int *)(extraout_ECX + 0xb4) * 4);
          *(undefined4 *)(extraout_ECX + 0xd8) =
               *(undefined4 *)(extraout_ECX + 0xfae4 + *(int *)(extraout_ECX + 0xb4) * 4);
          *(undefined4 *)(extraout_ECX + 0xdc) =
               *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4);
          *(undefined4 *)(extraout_ECX + 0xe0) =
               *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4);
          *(int *)(extraout_ECX + 0xb4) = DAT_00bd95cc;
        }
        if (((((((DAT_00bd95cc == 0x1d4d) || (DAT_00bd95cc == 0x1d4e)) ||
               ((DAT_00bd95cc == 0x1d53 || ((DAT_00bd95cc == 0x1d54 || (DAT_00bd95cc == 0x1d59))))))
              || (DAT_00bd95cc == 0x1d5a)) ||
             (((DAT_00bd95cc == 0x1d61 || (DAT_00bd95cc == 0x1d62)) || (DAT_00bd95cc == 0x1d67))))
            || ((DAT_00bd95cc == 0x1d68 || (DAT_00bd95cc == 0x1d6d)))) ||
           ((((DAT_00bd95cc == 0x1d6e || ((DAT_00bd95cc == 0x1d82 || (DAT_00bd95cc == 0x1d83)))) ||
             (DAT_00bd95cc == 0x1d88)) ||
            ((((((DAT_00bd95cc == 0x1d89 || (DAT_00bd95cc == 0x1d93)) || (DAT_00bd95cc == 0x1d94))
               || ((DAT_00bd95cc == 0x1d99 || (DAT_00bd95cc == 0x1d9a)))) ||
              ((DAT_00bd95cc == 0x1d9f || ((DAT_00bd95cc == 0x1da0 || (DAT_00bd95cc == 0x1da5))))))
             || (DAT_00bd95cc == 0x1da6)))))) {
          Ordinal_3312();
          *(undefined4 *)(extraout_ECX + 0xdba4 + *(int *)(extraout_ECX + 0xb4) * 4) = 0xffffffee;
          *(undefined4 *)(extraout_ECX + 0xfae4 + *(int *)(extraout_ECX + 0xb4) * 4) = 0xfffffffd;
          *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 0xf;
          *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0xf;
          *(undefined4 *)(extraout_ECX + 0xd4) =
               *(undefined4 *)(extraout_ECX + 0xdba4 + *(int *)(extraout_ECX + 0xb4) * 4);
          *(undefined4 *)(extraout_ECX + 0xd8) =
               *(undefined4 *)(extraout_ECX + 0xfae4 + *(int *)(extraout_ECX + 0xb4) * 4);
          *(undefined4 *)(extraout_ECX + 0xdc) =
               *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4);
          *(undefined4 *)(extraout_ECX + 0xe0) =
               *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4);
          *(int *)(extraout_ECX + 0xb4) = DAT_00bd95cc;
        }
        if ((((((DAT_00bd95cc == 0x1d4f) || (DAT_00bd95cc == 0x1d50)) || (DAT_00bd95cc == 0x1d51))
             || (((DAT_00bd95cc == 0x1d55 || (DAT_00bd95cc == 0x1d56)) ||
                 ((DAT_00bd95cc == 0x1d57 || ((DAT_00bd95cc == 0x1d5b || (DAT_00bd95cc == 0x1d5c))))
                 )))) ||
            ((((DAT_00bd95cc == 0x1d5d ||
               ((((DAT_00bd95cc == 0x1d63 || (DAT_00bd95cc == 0x1d64)) || (DAT_00bd95cc == 0x1d65))
                || ((DAT_00bd95cc == 0x1d69 || (DAT_00bd95cc == 0x1d6a)))))) ||
              (((DAT_00bd95cc == 0x1d6b || ((DAT_00bd95cc == 0x1d6f || (DAT_00bd95cc == 0x1d70))))
               || (DAT_00bd95cc == 0x1d71)))) ||
             (((((DAT_00bd95cc == 0x1d80 || (DAT_00bd95cc == 0x1d84)) || (DAT_00bd95cc == 0x1d85))
               || ((DAT_00bd95cc == 0x1d86 || (DAT_00bd95cc == 0x1d8a)))) ||
              (DAT_00bd95cc == 0x1d8b)))))) ||
           ((((DAT_00bd95cc == 0x1d8c || (DAT_00bd95cc == 0x1d95)) ||
             ((DAT_00bd95cc == 0x1d96 ||
              (((DAT_00bd95cc == 0x1d97 || (DAT_00bd95cc == 0x1d9b)) || (DAT_00bd95cc == 0x1d9c)))))
             ) || (((DAT_00bd95cc == 0x1d9d || (DAT_00bd95cc == 0x1da1)) ||
                   ((DAT_00bd95cc == 0x1da2 ||
                    (((DAT_00bd95cc == 0x1da3 || (DAT_00bd95cc == 0x1da7)) ||
                     ((DAT_00bd95cc == 0x1da8 || (DAT_00bd95cc == 0x1da9)))))))))))) {
          Ordinal_3312();
          *(undefined4 *)(extraout_ECX + 0xdba4 + *(int *)(extraout_ECX + 0xb4) * 4) = 0xffffffe2;
          *(undefined4 *)(extraout_ECX + 0xfae4 + *(int *)(extraout_ECX + 0xb4) * 4) = 0xfffffffd;
          *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4) = 9;
          *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 9;
          *(undefined4 *)(extraout_ECX + 0xd4) =
               *(undefined4 *)(extraout_ECX + 0xdba4 + *(int *)(extraout_ECX + 0xb4) * 4);
          *(undefined4 *)(extraout_ECX + 0xd8) =
               *(undefined4 *)(extraout_ECX + 0xfae4 + *(int *)(extraout_ECX + 0xb4) * 4);
          *(undefined4 *)(extraout_ECX + 0xdc) =
               *(undefined4 *)(extraout_ECX + 0x11a24 + *(int *)(extraout_ECX + 0xb4) * 4);
          *(undefined4 *)(extraout_ECX + 0xe0) =
               *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4);
          *(int *)(extraout_ECX + 0xb4) = DAT_00bd95cc;
        }
        if (DAT_00bd95cc == 0x420) {
          sprintf(acStackY_10500,"%s");
          chkesp();
          if (cStackY_104fe == '0') {
            Ordinal_3312();
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x21;
            *(undefined4 *)(extraout_ECX + 0xe0) =
                 *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4);
            *(int *)(extraout_ECX + 0xb4) = DAT_00bd95cc;
          }
          else {
            Ordinal_3312();
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x24;
            *(undefined4 *)(extraout_ECX + 0xe0) =
                 *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4);
            *(int *)(extraout_ECX + 0xb4) = DAT_00bd95cc;
          }
        }
        if (DAT_00bd95cc == 0x425) {
          sprintf(acStackY_10534,"%s");
          chkesp();
          if (acStackY_10534[0] == '0') {
            Ordinal_3312();
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x21;
            *(undefined4 *)(extraout_ECX + 0xe0) =
                 *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4);
            *(int *)(extraout_ECX + 0xb4) = DAT_00bd95cc;
          }
          else {
            Ordinal_3312();
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x24;
            *(undefined4 *)(extraout_ECX + 0xe0) =
                 *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4);
            *(int *)(extraout_ECX + 0xb4) = DAT_00bd95cc;
          }
        }
        if ((9999 < DAT_00bd95cc) && (DAT_00bd95cc < 0x2732)) {
          Ordinal_3312();
          if (DAT_00bd95cc == 10000) {
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0xf;
          }
          if (DAT_00bd95cc == 0x2711) {
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x15;
          }
          if (DAT_00bd95cc == 0x2712) {
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1b;
          }
          if (DAT_00bd95cc == 0x2713) {
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x21;
          }
          if (DAT_00bd95cc == 0x271a) {
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0xf;
          }
          if (DAT_00bd95cc == 0x271b) {
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x15;
          }
          if (DAT_00bd95cc == 0x271c) {
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1b;
          }
          if (DAT_00bd95cc == 0x271d) {
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x21;
          }
          if (DAT_00bd95cc == 0x2724) {
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0xf;
          }
          if (DAT_00bd95cc == 0x2725) {
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x15;
          }
          if (DAT_00bd95cc == 0x2726) {
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1b;
          }
          if (DAT_00bd95cc == 0x2727) {
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x21;
          }
          if (DAT_00bd95cc == 0x272e) {
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0xf;
          }
          if (DAT_00bd95cc == 0x272f) {
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x15;
          }
          if (DAT_00bd95cc == 0x2730) {
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x1b;
          }
          if (DAT_00bd95cc == 0x2731) {
            *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4) = 0x21;
          }
          *(undefined4 *)(extraout_ECX + 0xe0) =
               *(undefined4 *)(extraout_ECX + 0x13964 + *(int *)(extraout_ECX + 0xb4) * 4);
          *(int *)(extraout_ECX + 0xb4) = DAT_00bd95cc;
        }
        Ordinal_3312();
      }
    }
  }
  if (*(int *)(extraout_ECX + 0xa1b60) == 1) {
    for (iStackY_10538 = 0; iStackY_10538 < *(int *)(extraout_ECX + 0xbc);
        iStackY_10538 = iStackY_10538 + 1) {
      if ((0 < *(int *)(extraout_ECX + 0x2024 + iStackY_10538 * 4)) &&
         (*(int *)(extraout_ECX + 0x158a4 + iStackY_10538 * 4) == 0)) {
        if (((*(int *)(extraout_ECX + 0x2024 + iStackY_10538 * 4) == 0x420) &&
            (((iStackY_105d8 =
                    *(int *)(extraout_ECX + 0x5ea4 + iStackY_10538 * 4) + 3 +
                    *(int *)(extraout_ECX + 0xdba4 + iStackY_10538 * 4),
              (double)iStackY_105d8 <= (double)param_2 / *(double *)(iVar6 + 0x58) &&
              (iStackY_105dc =
                    *(int *)(extraout_ECX + 0x7de4 + iStackY_10538 * 4) + 1 +
                    *(int *)(extraout_ECX + 0xfae4 + iStackY_10538 * 4),
              (double)iStackY_105dc <= (double)param_3 / *(double *)(iVar6 + 0x58))) &&
             (iStackY_105e0 =
                   *(int *)(extraout_ECX + 0x5ea4 + iStackY_10538 * 4) + -1 +
                   *(int *)(extraout_ECX + 0x11a24 + iStackY_10538 * 4),
             (double)param_2 / *(double *)(iVar6 + 0x58) <= (double)iStackY_105e0)))) &&
           (iStackY_105e4 =
                 *(int *)(extraout_ECX + 0x7de4 + iStackY_10538 * 4) + -1 +
                 *(int *)(extraout_ECX + 0x13964 + iStackY_10538 * 4),
           (double)param_3 / *(double *)(iVar6 + 0x58) <= (double)iStackY_105e4)) {
          if (DAT_00bd955c != (int *)0x0) {
            (**(code **)(*DAT_00bd955c + 0x60))();
            chkesp();
          }
          sprintf(acStackY_1056c,"%s");
          chkesp();
          sprintf(acStackY_10574,"");
          chkesp();
          sprintf(acStackY_10574,"%c%c%c");
          chkesp();
          atof(acStackY_10574);
          fVar9 = (float10)chkesp();
          DAT_00bd943c = (float)fVar9;
          sprintf(acStackY_10574,"%c%c%c");
          chkesp();
          atof(acStackY_10574);
          fVar9 = (float10)chkesp();
          DAT_00bd9438 = (float)fVar9;
          _DAT_00bd944c = DAT_00bd943c;
          _DAT_00bd9448 = DAT_00bd9438;
          Ordinal_734();
          _DAT_00bd9450 =
               (float)*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xc0) * 4);
          if (_DAT_00bd9450 <= DAT_00bd943c) {
            _DAT_00bd9450 = DAT_00bd943c;
          }
          iStackY_105c0 = Ordinal_719();
          local_8 = 0x38;
          if (iStackY_105c0 == 0) {
            piStackY_105e8 = (int *)0x0;
          }
          else {
            piStackY_105e8 = (int *)thunk_FUN_008cca60();
          }
          piStackY_105bc = piStackY_105e8;
          local_8 = 0xffffffff;
          DAT_00bd955c = piStackY_105e8;
          Ordinal_4951();
        }
        if (((*(int *)(extraout_ECX + 0x2024 + iStackY_10538 * 4) == 0x425) &&
            (iStackY_105ec =
                  *(int *)(extraout_ECX + 0x5ea4 + iStackY_10538 * 4) + 3 +
                  *(int *)(extraout_ECX + 0xdba4 + iStackY_10538 * 4),
            (double)iStackY_105ec <= (double)param_2 / *(double *)(iVar6 + 0x58))) &&
           ((iStackY_105f0 =
                  *(int *)(extraout_ECX + 0x7de4 + iStackY_10538 * 4) + 1 +
                  *(int *)(extraout_ECX + 0xfae4 + iStackY_10538 * 4),
            (double)iStackY_105f0 <= (double)param_3 / *(double *)(iVar6 + 0x58) &&
            ((iStackY_105f4 =
                   *(int *)(extraout_ECX + 0x5ea4 + iStackY_10538 * 4) + -1 +
                   *(int *)(extraout_ECX + 0x11a24 + iStackY_10538 * 4),
             (double)param_2 / *(double *)(iVar6 + 0x58) <= (double)iStackY_105f4 &&
             (iStackY_105f8 =
                   *(int *)(extraout_ECX + 0x7de4 + iStackY_10538 * 4) + -1 +
                   *(int *)(extraout_ECX + 0x13964 + iStackY_10538 * 4),
             (double)param_3 / *(double *)(iVar6 + 0x58) <= (double)iStackY_105f8)))))) {
          if (DAT_00bd9558 != (int *)0x0) {
            (**(code **)(*DAT_00bd9558 + 0x60))();
            chkesp();
          }
          sprintf(acStackY_105a8,"%s");
          chkesp();
          sprintf(acStackY_105b0,"");
          chkesp();
          sprintf(acStackY_105b0,"%c%c%c");
          chkesp();
          atof(acStackY_105b0);
          fVar9 = (float10)chkesp();
          DAT_00bd9434 = (float)fVar9;
          sprintf(acStackY_105b0,"%c%c%c");
          chkesp();
          atof(acStackY_105b0);
          fVar9 = (float10)chkesp();
          DAT_00bd9430 = (float)fVar9;
          _DAT_00bd9444 = DAT_00bd9434;
          _DAT_00bd9440 = DAT_00bd9430;
          Ordinal_734();
          _DAT_00bd9450 =
               (float)*(int *)(extraout_ECX + 0x177e4 + *(int *)(extraout_ECX + 0xc0) * 4);
          iStackY_105c8 = Ordinal_719();
          local_8 = 0x39;
          if (iStackY_105c8 == 0) {
            piStackY_105fc = (int *)0x0;
          }
          else {
            piStackY_105fc = (int *)thunk_FUN_008cd150();
          }
          piStackY_105c4 = piStackY_105fc;
          local_8 = 0xffffffff;
          DAT_00bd9558 = piStackY_105fc;
          Ordinal_4951();
        }
      }
    }
  }
  Ordinal_3862();
LAB_0044bd9a:
  ExceptionList = local_10;
  chkesp();
  return;
}


