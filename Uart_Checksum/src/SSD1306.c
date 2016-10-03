#include <SSD1306.h>

GPIO_InitTypeDef G;
SPI_InitTypeDef S;

uint8_t GBuf[GBufS];

void SSD1306_InitSetup(void){
	static uint8_t Init = 1;
	if(Init == 1){
		Init = 0;
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

		G.GPIO_Pin = DC | RS;
		G.GPIO_Mode = GPIO_Mode_OUT;
		G.GPIO_OType = GPIO_OType_PP;
		G.GPIO_PuPd = GPIO_PuPd_UP;
		G.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(IOGPIO, &G);

		G.GPIO_Pin = CE;
		GPIO_Init(CEGPIO, &G);

		G.GPIO_Pin = Clk | DIn;
		G.GPIO_Mode = GPIO_Mode_AF;
		GPIO_Init(IOGPIO, &G);

		GPIO_PinAFConfig(IOGPIO, ClkPS, SPIAF);
		GPIO_PinAFConfig(IOGPIO, DInPS, SPIAF);

		S.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
		S.SPI_CPHA = SPI_CPHA_1Edge;
		S.SPI_CPOL = SPI_CPOL_Low;
		S.SPI_DataSize = SPI_DataSize_8b;
		S.SPI_Direction = SPI_Direction_1Line_Tx;
		S.SPI_FirstBit = SPI_FirstBit_MSB;
		S.SPI_Mode = SPI_Mode_Master;
		S.SPI_NSS = SPI_NSS_Soft;
		SPI_Init(SPI1, &S);
		SPI_Cmd(SPI1, ENABLE);
	}

	GPIO_ResetBits(IOGPIO, DC|CE|RS);
	Delay(1);
	GPIO_SetBits(IOGPIO, RS);
	Delay(1);
	GPIO_ResetBits(IOGPIO, RS);
	Delay(1);
	GPIO_SetBits(IOGPIO, RS|DC|CE);

	SB(SetMuxRatio, 0, 1);
	SB(0x3F, 0, 1);
	SB(SetDispOffset, 0, 1);
	SB(0x00, 0, 1);
	SB(SetDispStartLine|0, 0, 1);
	SB(SetRemap|1, 0, 1);
	SB(SetComPinHW, 0, 1);
	SB(0x12, 0, 1);
	SB(SetComScanDir|8, 0, 1);

	SB(Contrast, 0, 1);
	SB(0x7F, 0, 1);
	SB(MemAddMode, 0, 1);
	SB(MModeH, 0, 1);
	SB(SetColAdd, 0, 1);
	SB(0x00, 0, 1);
	SB(0x7F, 0, 1);
	SB(SetPageAdd, 0, 1);
	SB(0x00, 0, 1);
	SB(0x07, 0, 1);

	SB(NormDisp, 0, 1);
	SB(SetComHLvl, 0, 1);
	SB(0x40, 0, 1);

	//SB(DispOnAll, 0, 1); //Test whole display
	SB(DispOnRAM, 0, 1);
	SB(SetDispFreq, 0, 1);
	SB(0x80, 0, 1);
	SB(ChargePump, 0, 1);
	SB(0x14, 0, 1);
	SB(DispOn, 0, 1);

	ClrBuf();
	PScrn();
}

void TestScreenMode(uint8_t Mode){
	if(Mode==DispOnRAM) SB(DispOnRAM, 0, 1);
	else SB(DispOnAll, 0, 1);
}

void SB(uint8_t Dat, uint8_t CmdDat, uint8_t En){
	GPIO_WriteBit(IOGPIO, DC, CmdDat);
	if(En) GPIO_ResetBits(IOGPIO, CE);

	SPI_I2S_SendData(SPI1, Dat);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);

	if(En) GPIO_SetBits(IOGPIO, CE);
}

void InvertMode(void){
	SB(InvDisp, 0, 1);
}

void NormalMode(void){
	SB(NormDisp, 0, 1);
}

void PScrn(void){
	uint16_t Cnt;
	//GPIO_ResetBits(IOGPIO, CE);
	for(Cnt = 0; Cnt<GBufS; Cnt++){
		SB(GBuf[Cnt], 1, 1);
	}
	//GPIO_SetBits(IOGPIO, CE);
}
