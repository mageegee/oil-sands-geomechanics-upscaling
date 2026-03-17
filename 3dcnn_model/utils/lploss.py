import torch 

#loss function with rel/abs Lp loss
class LpLoss(object):
    def __init__(self, d=2, p=2, size_average=True, reduction=True, LGR=False, mode='sat'):
        super(LpLoss, self).__init__()

        #Dimension and Lp-norm type are postive
        assert d > 0 and p > 0

        self.d = d
        self.p = p
        self.reduction = reduction
        self.size_average = size_average
        self.mode = mode

    def rel(self, x, y):
        num_examples = x.size()[0]
        num_channels = x.size()[1]   # Number of channels (4 in our case)
        length = x.size()[2]         # Length (66 in our case)

        diff_norms = 0
        y_norms = 0
        
        total = []
    
        for channel in range(num_channels):
            diff_norm = torch.norm(x[:, channel, :].reshape(num_examples,-1) - y[:, channel, :].reshape(num_examples,-1), self.p, dim=1)
            y_norm = torch.norm(y[:, channel, :].reshape(num_examples,-1), self.p, dim=1) + 1e-8

            #diff_norms += diff_norm
            #y_norms += y_norm

            relative_loss = torch.mean(diff_norm / y_norm)
            absolute_loss = torch.mean(diff_norm)

            total.append(torch.mean(diff_norm/y_norm))
            # total.append(relative_loss + absolute_loss)

        #diff_norms = torch.norm(x.reshape(num_examples,-1) - y.reshape(num_examples,-1), self.p, 1)
        #y_norms = torch.norm(y.reshape(num_examples,-1), self.p, 1)

        return total
    
    def __call__(self, x, y):
        return self.rel(x, y)
    

import torch.nn as nn
def RMSE(pred, target):
    # Ensure pred and target have the same shape
    assert pred.shape == target.shape, "Predicted and target tensors must have the same shape"
    
    num_examples = target.size()[0]
    num_channels = target.size()[1]

    # List to store MSE loss for each timestep
    total = []
    
    for channel in range(num_channels):
        diff_norm = (pred[:, channel, ...].reshape(num_examples,-1) - target[:, channel, ...].reshape(num_examples,-1))**2
        y_norm = (target[:, channel, ...].reshape(num_examples,-1))**2 + 1e-8

        mse = nn.MSELoss()
        total.append(torch.sqrt(mse(pred[:, channel, ...], target[:, channel, ...])))

        # total.append(torch.mean(diff_norm))#.sqrt())
        # total.append(torch.mean(diff_norm/y_norm))

    return total