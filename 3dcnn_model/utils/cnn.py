import torch
import torch.nn as nn
import torch.nn.functional as F
import numpy as np


class CNN(nn.Module):
    def __init__(self):
        super().__init__()

        # (Batch, 1, 20, 20, 20)
        self.conv1 = nn.Conv3d(in_channels=1, out_channels=128, kernel_size=(5, 5, 5), stride=(1, 1, 1), padding=2)
        self.bn1 = nn.BatchNorm3d(num_features=128)
        self.pool1 = nn.MaxPool3d(kernel_size=2, stride=2,)

        # (Batch, 96, 10, 10, 10)
        self.conv2 = nn.Conv3d(in_channels=128, out_channels=256, kernel_size=(5, 5, 5), stride=(1, 1, 1), padding=2)
        self.bn2 = nn.BatchNorm3d(num_features=256)
        self.pool2 = nn.MaxPool3d(kernel_size=2, stride=2,)

        # (Batch, 256, 5, 5, 5)
        self.conv3 = nn.Conv3d(in_channels=256, out_channels=512, kernel_size=(5, 5, 5), stride=(1, 1, 1), padding=2)
        self.bn3 = nn.BatchNorm3d(num_features=512)
        self.conv4 = nn.Conv3d(in_channels=512, out_channels=512, kernel_size=(5, 5, 5), stride=(1, 1, 1), padding=2)
        self.bn4 = nn.BatchNorm3d(num_features=512)

        # (Batch, 384, 5, 5, 5)
        self.conv5 = nn.Conv3d(in_channels=512, out_channels=1024, kernel_size=(5, 5, 5), stride=(1, 1, 1), padding=2)
        self.bn5 = nn.BatchNorm3d(num_features=1024)
        self.pool5 = nn.MaxPool3d(kernel_size=2, stride=2,)
        
        # (Batch, 256, 2, 2, 2) - Flatten

        # (Batch, 256, 8)
        self.deconv1_1 = nn.ConvTranspose1d(in_channels=256, out_channels=512, kernel_size=3, stride=3, padding=1)
        self.debn1_1 = nn.BatchNorm1d(num_features=512)
        # (Batch, 1024, 22)
        self.deconv1_2 = nn.ConvTranspose1d(in_channels=512, out_channels=1024, kernel_size=3, stride=3, padding=0)
        self.debn1_2 = nn.BatchNorm1d(num_features=1024)

        # (Batch, 256, 8)
        self.deconv2_1 = nn.ConvTranspose1d(in_channels=256, out_channels=512, kernel_size=3, stride=3, padding=1)
        self.debn2_1 = nn.BatchNorm1d(num_features=512)
        # (Batch, 1024, 22)
        self.deconv2_2 = nn.ConvTranspose1d(in_channels=512, out_channels=1024, kernel_size=3, stride=3, padding=0)
        self.debn2_2 = nn.BatchNorm1d(num_features=1024)

        # (Batch, 256, 8)
        self.deconv3_1 = nn.ConvTranspose1d(in_channels=256, out_channels=512, kernel_size=3, stride=3, padding=1)
        self.debn3_1 = nn.BatchNorm1d(num_features=512)
        # (Batch, 1024, 22)
        self.deconv3_2 = nn.ConvTranspose1d(in_channels=512, out_channels=1024, kernel_size=3, stride=3, padding=0)
        self.debn3_2 = nn.BatchNorm1d(num_features=1024)

        # (Batch, 256, 8)
        self.deconv4_1 = nn.ConvTranspose1d(in_channels=256, out_channels=512, kernel_size=3, stride=3, padding=1)
        self.debn4_1 = nn.BatchNorm1d(num_features=512)
        # (Batch, 1024, 22)
        self.deconv4_2 = nn.ConvTranspose1d(in_channels=512, out_channels=1024, kernel_size=3, stride=3, padding=0)
        self.debn4_2 = nn.BatchNorm1d(num_features=1024)

        # (Batch, 1024, 65)
        self.ln1_1 = nn.Linear(in_features=1024*8, out_features=1024*4)
        self.ln1_2 = nn.Linear(in_features=1024*4, out_features=1024*2)
        self.ln1_3 = nn.Linear(in_features=1024*2, out_features=65)
        self.drop1_1 = nn.Dropout(0.5)
        self.drop1_2 = nn.Dropout(0.5)

        self.ln2_1 = nn.Linear(in_features=1024*8, out_features=1024*4)
        self.ln2_2 = nn.Linear(in_features=1024*4, out_features=1024*2)
        self.ln2_3 = nn.Linear(in_features=1024*2, out_features=65)
        self.drop2_1 = nn.Dropout(0.5)
        self.drop2_2 = nn.Dropout(0.5)

        self.ln3_1 = nn.Linear(in_features=1024*8, out_features=1024*4)
        self.ln3_2 = nn.Linear(in_features=1024*4, out_features=1024*2)
        self.ln3_3 = nn.Linear(in_features=1024*2, out_features=65)
        self.drop3_1 = nn.Dropout(0.5)
        self.drop3_2 = nn.Dropout(0.5)

        self.ln4_1 = nn.Linear(in_features=1024*8, out_features=1024*4)
        self.ln4_2 = nn.Linear(in_features=1024*4, out_features=1024*2)
        self.ln4_3 = nn.Linear(in_features=1024*2, out_features=65)
        self.drop4_1 = nn.Dropout(0.5)
        self.drop4_2 = nn.Dropout(0.5)


    def forward(self, x):
        x = self.conv1(x)
        x = self.bn1(x)
        x = F.gelu(x)
        x = self.pool1(x)

        x = self.conv2(x)
        x = self.bn2(x)
        x = F.gelu(x)
        x = self.pool2(x)

        x = self.conv3(x)
        x = self.bn3(x)
        x = F.gelu(x)

        x = self.conv4(x)
        x = self.bn4(x)
        x = F.gelu(x)

        x = self.conv5(x)        
        x = self.bn5(x)
        x = F.gelu(x)
        x = self.pool5(x)

        x = torch.flatten(x, start_dim=1)

        x1 = x
        x2 = x
        x3 = x
        x4 = x


        x1 = self.ln1_1(x1)
        x1 = F.gelu(x1)
        x1 = self.drop1_1(x1)
        x1 = self.ln1_2(x1)
        x1 = F.gelu(x1)
        x1 = self.drop1_2(x1)
        x1 = self.ln1_3(x1)

        x2 = self.ln2_1(x2)
        x2 = F.gelu(x2)
        x2 = self.drop2_1(x2)
        x2 = self.ln2_2(x2)
        x2 = F.gelu(x2)
        x2 = self.drop2_2(x2)
        x2 = self.ln2_3(x2)
        
        x3 = self.ln3_1(x3)
        x3 = F.gelu(x3)
        x3 = self.drop3_1(x3)
        x3 = self.ln3_2(x3)
        x3 = F.gelu(x3)
        x3 = self.drop3_2(x3)
        x3 = self.ln3_3(x3)

        x4 = self.ln4_1(x4)
        x4 = F.gelu(x4)
        x4 = self.drop4_1(x4)
        x4 = self.ln4_2(x4)
        x4 = F.gelu(x4)
        x4 = self.drop4_2(x4)
        x4 = self.ln4_3(x4)
        
        # x = torch.cat((x1,x2,x3,x4),1)
        x = torch.stack([x1,x2,x3,x4], dim=-1)
        
        x = x.permute(0, 2, 1) # (B, C, L)

        return x




