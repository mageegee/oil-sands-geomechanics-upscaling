import torch
import torch.nn as nn
import torch.nn.functional as F

from .layers import *
from .lstm_one import *


class RRUNet(nn.Module):
    """
    Full set of 3D Recurrent Residual U-Net.
    Consists of 3 complete layer:
        1. Encoder
        2. Bottleneck
        3. Decoder

    Args:
        Args.

    """

    def __init__(self, depth, hidden_channel=16, input_channel=1):
        super().__init__()
        self.depth = depth
    
        # Instance layers
        # Repeat dim layer
        self.time_repeat = repeat_conv(self.depth)

        # Encoder layers
        # (N,1,D,H,W)
        self.enc1 = conv_block(input_channel, hidden_channel, stride=2)
        # (N,16,D/2,H/2,W/2)
        self.enc2 = conv_block(hidden_channel, hidden_channel * 2, stride=1)
        # (N,32,D/2,H/2,W/2)
        self.enc3 = conv_block(hidden_channel * 2, hidden_channel * 2, stride=2)
        # (N,32,D/4,H/4,W/4)
        self.enc4 = conv_block(hidden_channel * 2, hidden_channel * 4, stride=1)
        # (N,64,D/4,H/4,W/4)


        # Bottleneck Layers
        # (N,64,D/4,H/4,W/4)
        self.res1 = res_block(hidden_channel * 4, hidden_channel * 4)
        # (N,64,D/4,H/4,W/4)
        self.res2 = res_block(hidden_channel * 4, hidden_channel * 4)
        # (N,64,D/4,H/4,W/4)

        # (N,time,64,D/4,H/4,W/4)
        self.lstm = ConvLSTM(hidden_channel * 4, hidden_channel * 4, (3,3,3), 1, True, True, False)
        # (N,time,64,D/4,H/4,W/4)

        # (N,time,64,D/4,H/4,W/4)
        self.time_res1 = time_res_block(hidden_channel * 4, hidden_channel * 4)
        # (N,time,64,D/4,H/4,W/4)
        self.time_res2 = time_res_block(hidden_channel * 4, hidden_channel * 4)
        # (N,time,64,D/4,H/4,W/4)


        # Decoder layers
        # (N,time,128,D/4,H/4,W/4)
        self.deconv4 = time_deconv_block(hidden_channel * 8, hidden_channel * 4, stride=(1,1,1))
        # (N,time,96,D/4,H/4,W/4)
        self.deconv3 = time_deconv_block(hidden_channel * 6, hidden_channel * 2, stride=(2,2,2))
        # (N,time,64,D/2,H/2,W/2)
        self.deconv2 = time_deconv_block(hidden_channel * 4, hidden_channel * 2, stride=(1,1,1))
        # (N,time,48,D/2,H/2,W/2)
        self.deconv1 = time_deconv_block(hidden_channel * 3, hidden_channel, stride=(2,2,2))
        # (N,time,16,D,H,W)

        # (N,time,16,D,H,W)
        self.output = TimeDistributed(nn.Conv3d(hidden_channel, 1, 3, padding=1))
        #self.output = time_LFS_output_block(16, 1, (12,40,40))#(55,103,99)
        # (N,time,1,D,H,W)

    def forward(self, x):
        # Encoder
        # (N,1,D,H,W)
        enc1 = self.enc1(x)
        # (N,16,D/2,H/2,W/2)
        time_enc1 = self.time_repeat(enc1)
        # (N,time,16,D/2,H/2,W/2)

        # (N,16,D/2,H/2,W/2)
        enc2 = self.enc2(enc1)
        # (N,32,D/2,H/2,W/2)
        time_enc2 = self.time_repeat(enc2)
        # (N,time,32,D/2,H/2,W/2)

        # (N,32,D/2,H/2,W/2)
        enc3 = self.enc3(enc2)
        # (N,32,D/4,H/4,W/4)
        time_enc3 = self.time_repeat(enc3)
        # (N,time,32,D/4,H/4,W/4)

        # (N,32,D/4,H/4,W/4)
        enc4 = self.enc4(enc3)
        # (N,64,D/4,H/4,W/4)
        time_enc4 = self.time_repeat(enc4)
        # (N,time,64,D/4,H/4,W/4)


        # Bottleneck
        # (N,64,D/4,H/4,W/4)
        res1 = self.res1(enc4)
        # (N,64,D/4,H/4,W/4)
        res2 = self.res2(res1)
        # (N,64,D/4,H/4,W/4)

        # (N,64,D/4,H/4,W/4)
        res2 = self.time_repeat(res2)
        # (N,time,64,D/4,H/4,W/4)

        # (N,time,64,D/4,H/4,W/4)
        lstm, _ = self.lstm(res2)
        # (N,time,64,D/4,H/4,W/4)

        # (N,time,64,D/4,H/4,W/4)
        time_res1 = self.time_res1(lstm)
        # (N,time,64,D/4,H/4,W/4)
        time_res2 = self.time_res2(time_res1)
        # (N,time,64,D/4,H/4,W/4)


        # Decoder
        # (N,time,64+64,D/4,H/4,W/4)
        merge4 = torch.cat([time_enc4,time_res2], dim=2)
        # (N,time,128,D/4,H/4,W/4)
        dec4 = self.deconv4(merge4)
        # (N,time,64,D/4,H/4,W/4)

        # (N,time,32+64,D/4,H/4,W/4)
        merge3 = torch.cat([time_enc3,dec4], dim=2)
        # (N,time,96,D/4,H/4,W/4)
        dec3 = self.deconv3(merge3)
        # (N,time,32,D/2,H/2,W/2)

        # (N,time,32+32,D/2,H/2,W/2)
        merge2 = torch.cat([time_enc2,dec3], dim=2)
        # (N,time,64,D/2,H/2,W/2)
        dec2 = self.deconv2(merge2)
        # (N,time,32,D/2,H/2,W/2)

        # (N,time,16+32,D/2,H/2,W/2)
        merge1 = torch.cat([time_enc1,dec2], dim=2)
        # (N,time,48,D/2,H/2,W/2)
        dec1 = self.deconv1(merge1)
        # (N,time,16,D,H,W)

        # (N,time,16,D,H,W)
        output = self.output(dec1)
        # (N,time,1,nD,nH,nW)

        return output