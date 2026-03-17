import torch
import torch.nn as nn

class WeightedLpLoss:
    """
    Weighted relative Lp loss for multi-channel outputs.
    Emphasizes specific channels by assigning higher weights.
    """
    
    def __init__(self, weights=None, p=2):
        """
        Args:
            weights: List of weights for each channel, e.g., [1.0, 1.0, 1.0, 2.5]
            p: Norm type (default: 2)
        """
        self.p = p
        
        if weights is None:
            self.weights = self.weights = torch.tensor([1, 1, 1, 1], dtype=torch.float32)
        else:
            self.weights = torch.tensor(weights, dtype=torch.float32)
        
        # Normalize to sum to num_channels
        num_channels = len(self.weights)
        self.weights = self.weights * num_channels / self.weights.sum()
        
        print(f"📊 Weighted loss initialized: {self.weights.tolist()}")
     

 
    def __call__(self, pred, target):
        """Compute weighted loss per channel."""
        num_examples = pred.size(0)
        num_channels = pred.size(1)
        
        losses = []
        
        for i in range(num_channels):
            pred_flat = pred[:, i, :].reshape(num_examples, -1)
            target_flat = target[:, i, :].reshape(num_examples, -1)
            
            diff_norm = torch.norm(pred_flat - target_flat, p=self.p, dim=1)
            target_norm = torch.norm(target_flat, p=self.p, dim=1) + 1e-8
            
            relative_loss = torch.mean(diff_norm / target_norm)
            weighted_loss = relative_loss * self.weights[i].to(pred.device)
            
            losses.append(weighted_loss)
        
        return losses