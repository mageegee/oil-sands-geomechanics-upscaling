import torch
import numpy as np
import torch.nn as nn
import torch.nn.functional as F
'''
import operator
from functools import reduce
from functools import partial

from timeit import default_timer

torch.manual_seed(0)
np.random.seed(0)
'''
class SpectralConv4d(nn.Module):
    def __init__(self, in_channels, out_channels, modes1, modes2, modes3, modes4):
        super(SpectralConv4d, self).__init__()

        """
        4D Fourier layer. It does FFT, linear transform, and Inverse FFT.    
        """

        self.in_channels = in_channels
        self.out_channels = out_channels
        self.modes1 = modes1 #Number of Fourier modes to multiply, at most floor(N/2) + 1
        self.modes2 = modes2
        self.modes3 = modes3
        self.modes4 = modes4
        
        self.scale = (1 / (in_channels * out_channels))
        self.weights1 = nn.Parameter(self.scale * torch.rand(in_channels, out_channels, self.modes1, self.modes2, self.modes3, self.modes4, dtype=torch.cfloat))
        self.weights2 = nn.Parameter(self.scale * torch.rand(in_channels, out_channels, self.modes1, self.modes2, self.modes3, self.modes4, dtype=torch.cfloat))
        self.weights3 = nn.Parameter(self.scale * torch.rand(in_channels, out_channels, self.modes1, self.modes2, self.modes3, self.modes4, dtype=torch.cfloat))
        self.weights4 = nn.Parameter(self.scale * torch.rand(in_channels, out_channels, self.modes1, self.modes2, self.modes3, self.modes4, dtype=torch.cfloat))
        self.weights5 = nn.Parameter(self.scale * torch.rand(in_channels, out_channels, self.modes1, self.modes2, self.modes3, self.modes4, dtype=torch.cfloat))
        self.weights6 = nn.Parameter(self.scale * torch.rand(in_channels, out_channels, self.modes1, self.modes2, self.modes3, self.modes4, dtype=torch.cfloat))
        self.weights7 = nn.Parameter(self.scale * torch.rand(in_channels, out_channels, self.modes1, self.modes2, self.modes3, self.modes4, dtype=torch.cfloat))
        self.weights8 = nn.Parameter(self.scale * torch.rand(in_channels, out_channels, self.modes1, self.modes2, self.modes3, self.modes4, dtype=torch.cfloat))

    # Complex multiplication
    def compl_mul4d(self, input, weights):
        # (batch, in_channel, x,y,t ), (in_channel, out_channel, x,y,t) -> (batch, out_channel, x,y,t)
        return torch.einsum("bixyzt,ioxyzt->boxyzt", input, weights)

    def forward(self, x):
        batchsize = x.shape[0]
        #Compute Fourier coeffcients up to factor of e^(- something constant)
        x_ft = torch.fft.rfftn(x, dim=[-4,-3,-2,-1])

        # Multiply relevant Fourier modes
        out_ft = torch.zeros(batchsize, self.out_channels, x.size(-4), x.size(-3), x.size(-2), x.size(-1)//2 + 1, dtype=torch.cfloat, device=x.device)

        out_ft[:, :, :self.modes1, :self.modes2, :self.modes3, :self.modes4] = self.compl_mul4d(x_ft[:, :, :self.modes1, :self.modes2, :self.modes3, :self.modes4], self.weights1)
        out_ft[:, :, -self.modes1:, :self.modes2, :self.modes3, :self.modes4] = self.compl_mul4d(x_ft[:, :, -self.modes1:, :self.modes2, :self.modes3, :self.modes4], self.weights2)
        out_ft[:, :, :self.modes1, -self.modes2:, :self.modes3, :self.modes4] = self.compl_mul4d(x_ft[:, :, :self.modes1, -self.modes2:, :self.modes3, :self.modes4], self.weights3)
        out_ft[:, :, :self.modes1, :self.modes2, -self.modes3:, :self.modes4] = self.compl_mul4d(x_ft[:, :, :self.modes1, :self.modes2, -self.modes3:, :self.modes4], self.weights4)
        out_ft[:, :, -self.modes1:, -self.modes2:, :self.modes3, :self.modes4] = self.compl_mul4d(x_ft[:, :, -self.modes1:, -self.modes2:, :self.modes3, :self.modes4], self.weights5)
        out_ft[:, :, -self.modes1:, :self.modes2, -self.modes3:, :self.modes4] = self.compl_mul4d(x_ft[:, :, -self.modes1:, :self.modes2, -self.modes3:, :self.modes4], self.weights6)
        out_ft[:, :, :self.modes1, -self.modes2:, -self.modes3:, :self.modes4] = self.compl_mul4d(x_ft[:, :, :self.modes1, -self.modes2:, -self.modes3:, :self.modes4], self.weights7)
        out_ft[:, :, -self.modes1:, -self.modes2:, -self.modes3:, :self.modes4] = self.compl_mul4d(x_ft[:, :, -self.modes1:, -self.modes2:, -self.modes3:, :self.modes4], self.weights8)        

        #Return to physical space
        x = torch.fft.irfftn(out_ft, s=(x.size(-4), x.size(-3), x.size(-2), x.size(-1)))
        return x

class Block4d(nn.Module):
    def __init__(self, width, width2, modes1, modes2, modes3, modes4, out_dim, layer, skip):
        super(Block4d, self).__init__()
        self.modes1 = modes1
        self.modes2 = modes2
        self.modes3 = modes3
        self.modes4 = modes4
        
        self.width = width
        self.width2 = width2
        self.out_dim = out_dim
        self.skip = skip
        self.padding = 8
        
        # channel
        self.conv0 = SpectralConv4d(self.width, self.width, self.modes1, self.modes2, self.modes3, self.modes4)
        self.conv1 = SpectralConv4d(self.width, self.width, self.modes1, self.modes2, self.modes3, self.modes4)
        self.conv2 = SpectralConv4d(self.width, self.width, self.modes1, self.modes2, self.modes3, self.modes4)
        self.conv3 = SpectralConv4d(self.width, self.width, self.modes1, self.modes2, self.modes3, self.modes4)
        
        self.w0 = nn.Conv1d(self.width, self.width, 1)
        self.w1 = nn.Conv1d(self.width, self.width, 1)
        self.w2 = nn.Conv1d(self.width, self.width, 1)
        self.w3 = nn.Conv1d(self.width, self.width, 1)
        '''
        self.w0 = nn.Conv2d(self.width, self.width, 1)
        self.w1 = nn.Conv2d(self.width, self.width, 1)
        self.w2 = nn.Conv2d(self.width, self.width, 1)
        self.w3 = nn.Conv2d(self.width, self.width, 1)
        '''
        
    def forward(self, x):
        batchsize = x.shape[0]
        size_t, size_z, size_x, size_y = x.shape[2], x.shape[3], x.shape[4], x.shape[5]

        # channel
        x1 = self.conv0(x)
        x2 = self.w0(x.view(batchsize, self.width, -1)).view(batchsize, self.width, size_t, size_z, size_x, size_y)
        #x2 = self.w0(x.view(batchsize, self.width, size_t, -1)).view(batchsize, self.width, size_t, size_z, size_x, size_y)
        if self.skip:
            x = x1 + x2 + x
        else:
            x = x1 + x2
        x = F.gelu(x)
        
        x1 = self.conv1(x)
        x2 = self.w1(x.view(batchsize, self.width, -1)).view(batchsize, self.width, size_t, size_z, size_x, size_y)
        #x2 = self.w1(x.view(batchsize, self.width, size_t, -1)).view(batchsize, self.width, size_t, size_z, size_x, size_y)
        if self.skip:
            x = x1 + x2 + x
        else:
            x = x1 + x2
        x = F.gelu(x)

        x1 = self.conv2(x)
        x2 = self.w2(x.view(batchsize, self.width, -1)).view(batchsize, self.width, size_t, size_z, size_x, size_y)
        #x2 = self.w2(x.view(batchsize, self.width, size_t, -1)).view(batchsize, self.width, size_t, size_z, size_x, size_y)
        if self.skip:
            x = x1 + x2 + x
        else:
            x = x1 + x2
        x = F.gelu(x)

        x1 = self.conv3(x)
        x2 = self.w3(x.view(batchsize, self.width, -1)).view(batchsize, self.width, size_t, size_z, size_x, size_y)
        #x2 = self.w3(x.view(batchsize, self.width, size_t, -1)).view(batchsize, self.width, size_t, size_z, size_x, size_y)
        if self.skip:
            x = x1 + x2 + x
        else:
            x = x1 + x2
        
        return x
    
class FNO4d(nn.Module):
    def __init__(self, modes1, modes2, modes3, modes4, width, in_dim=1, layer=None, skip=False):
        super(FNO4d, self).__init__()

        self.modes1 = modes1
        self.modes2 = modes2
        self.modes3 = modes3
        self.modes4 = modes4
        self.width = width
        self.width2 = width*4
        self.in_dim = in_dim
        self.out_dim = 1
        self.layer = layer
        self.skip = skip
        self.padding = 8  # pad the domain if input is non-periodic
        
        self.fc0 = nn.Linear(self.in_dim, self.width)
        self.conv = Block4d(self.width, self.width2, self.modes1, self.modes2, 
                            self.modes3, self.modes4, self.out_dim, self.layer, self.skip)
        self.fc1 = nn.Linear(self.width, self.width2)
        self.fc2 = nn.Linear(self.width2, self.out_dim)

    def forward(self, x, gradient=False):
        # Input shape: B T C Z X Y 
        #x = F.pad(x, [self.padding, self.padding, self.padding*2, self.padding*2, self.padding*2, 
        #              self.padding*2, self.padding, self.padding])
        x = x.permute(0, 1, 3, 4, 5, 2) # B T Z X Y C
        x = self.fc0(x)
        x = x.permute(0, 5, 1, 2, 3, 4) # B C T Z X Y

        #x = x.permute(0, 2, 1, 3, 4, 5) # B C T Z X Y
        x = F.pad(x, [self.padding, self.padding, self.padding, self.padding, self.padding, 
                      self.padding, self.padding, self.padding])
        
        #x = x.permute(0, 2, 3, 4, 5, 1) # B T Z X Y C
        #x = self.fc0(x) 
        #x = x.permute(0, 5, 1, 2, 3, 4) # B C T Z X Y
        
        
        #print(x.shape)
        x = self.conv(x)
           
        #x = x[:, :, self.padding:-self.padding, self.padding*2:-self.padding*2, 
        #      self.padding*2:-self.padding*2, self.padding:-self.padding]
        x = x[:, :, self.padding:-self.padding, self.padding:-self.padding, 
              self.padding:-self.padding, self.padding:-self.padding]
        
        x = x.permute(0, 2, 3, 4, 5, 1) # pad the domain if input is non-periodic
        x = self.fc1(x)
        x = F.gelu(x)
        x = self.fc2(x)

        x = x.permute(0, 1, 5, 2, 3, 4) # B T C X Y Z
    
        return x
    
    
    