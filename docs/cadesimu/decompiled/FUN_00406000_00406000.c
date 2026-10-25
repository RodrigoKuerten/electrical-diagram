// FUN_00406000 @ 00406000

/* WARNING: Removing unreachable block (ram,0x0040605f) */
/* WARNING: Removing unreachable block (ram,0x00406084) */

void FUN_00406000(void)

{
  char cVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 local_48 [16];
  int local_8;
  
  puVar3 = local_48;
  for (iVar2 = 0x11; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar3 = 0xcccccccc;
    puVar3 = puVar3 + 1;
  }
  Ordinal_3831();
  if (DAT_00bd95cc == 0x2777) {
    Ordinal_736(&DAT_00b462dc);
  }
  Ordinal_734(&DAT_00bd94a0);
  Ordinal_734(&DAT_00bd949c);
  *(undefined4 *)(local_8 + 0x90) = DAT_00bd95d0;
  *(undefined4 *)(local_8 + 0x94) = DAT_00bd95d4;
  *(undefined4 *)(local_8 + 0xa8) = DAT_00bd95f4;
  *(undefined4 *)(local_8 + 0x60) = 0xffffffff;
  *(undefined4 *)(local_8 + 100) = 0xffffffff;
  *(undefined4 *)(local_8 + 0x68) = 0xffffffff;
  cVar1 = Ordinal_813(&DAT_00bd9488,&DAT_00b462b0);
  if (cVar1 != '\0') {
    *(undefined4 *)(local_8 + 0x60) = 0;
  }
  cVar1 = Ordinal_813(&DAT_00bd9488,&DAT_00b462ac);
  if (cVar1 != '\0') {
    *(undefined4 *)(local_8 + 100) = 0;
  }
  cVar1 = Ordinal_813(&DAT_00bd9488,&DAT_00b462a8);
  if (cVar1 != '\0') {
    *(undefined4 *)(local_8 + 0x68) = 0;
  }
  *(undefined4 *)(local_8 + 0x6c) = 0xffffffff;
  *(undefined4 *)(local_8 + 0x70) = 0xffffffff;
  *(undefined4 *)(local_8 + 0x74) = 0xffffffff;
  Ordinal_734(&DAT_00bd9494);
  cVar1 = Ordinal_813(&DAT_00bd9484,&DAT_00b462b0);
  if (cVar1 != '\0') {
    *(undefined4 *)(local_8 + 0x6c) = 0;
  }
  cVar1 = Ordinal_813(&DAT_00bd9484,&DAT_00b462ac);
  if (cVar1 != '\0') {
    *(undefined4 *)(local_8 + 0x70) = 0;
  }
  cVar1 = Ordinal_813(&DAT_00bd9484,&DAT_00b462a8);
  if (cVar1 != '\0') {
    *(undefined4 *)(local_8 + 0x74) = 0;
  }
  *(undefined4 *)(local_8 + 0x78) = 0xffffffff;
  *(undefined4 *)(local_8 + 0x7c) = 0xffffffff;
  *(undefined4 *)(local_8 + 0x80) = 0xffffffff;
  Ordinal_734(&DAT_00bd948c);
  cVar1 = Ordinal_813(&DAT_00bd9480,&DAT_00b462b0);
  if (cVar1 != '\0') {
    *(undefined4 *)(local_8 + 0x78) = 0;
  }
  cVar1 = Ordinal_813(&DAT_00bd9480,&DAT_00b462ac);
  if (cVar1 != '\0') {
    *(undefined4 *)(local_8 + 0x7c) = 0;
  }
  cVar1 = Ordinal_813(&DAT_00bd9480,&DAT_00b462a8);
  if (cVar1 != '\0') {
    *(undefined4 *)(local_8 + 0x80) = 0;
  }
  *(undefined4 *)(local_8 + 0xa0) = 0xffffffff;
  cVar1 = Ordinal_813(&DAT_00bd947c,&DAT_00b462ac);
  if (cVar1 != '\0') {
    *(undefined4 *)(local_8 + 0xa0) = 0;
  }
  Ordinal_734(&DAT_00bd9474);
  Ordinal_5056(0);
  chkesp();
  return;
}


